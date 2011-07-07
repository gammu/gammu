///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1999/2000 Alexander Keck
//
//  $Workfile: UnicodeString.cpp $
//
//	$Author: Alex $
//
//	$Date: 21.12.99 13:02 $
//
//  Contents:	CUnicodeString - CUnicodeString Class
//
//  Classes:	CUnicodeString
//
//  Functions:	a lot
//
///////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UnicodeString.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define _wcsinc(_pc)    ((_pc)+1)

BOOL IsValidString(LPCSTR lpsz, int nLength)
{
	if(lpsz == NULL)
		return FALSE;
	return ::IsBadStringPtrA(lpsz, nLength) == 0;
}

BOOL IsValidString(LPCWSTR lpsz, int nLength)
{
	if(lpsz == NULL)
		return FALSE;
	return ::IsBadStringPtrW(lpsz, nLength) == 0;
}

CUnicodeString::CUnicodeString()
{
	m_Length = 0;
	m_MaximumLength = 0;
	m_Buffer = NULL;
	m_TempBuff = NULL;
}

CUnicodeString::CUnicodeString(UNICODE_STRING* puString)
{
	m_TempBuff = NULL;
	m_MaximumLength = m_Length = (puString->Length / sizeof(WCHAR));
	m_Buffer = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR));
	memset(m_Buffer,0x00,(m_Length + 1) * sizeof(WCHAR));
	memcpy(m_Buffer, puString->Buffer, m_Length * sizeof(WCHAR));
}

CUnicodeString::CUnicodeString(LPCWSTR pstrString)
{
	m_TempBuff = NULL;
	m_Buffer = NULL;
	InitializeFromWideString(pstrString);
}

CUnicodeString::CUnicodeString(LPCSTR pstrString)
{
	m_TempBuff = NULL;
	m_Buffer = NULL;
	InitializeFromString(pstrString);
}

CUnicodeString::CUnicodeString(const CUnicodeString& strString)
{
	if(strString.IsEmpty())
	{
		m_Length = m_MaximumLength = 0;
		m_Buffer = NULL;
		m_TempBuff = NULL;
	}
	else
	{
		m_Length = strString.m_Length;
		m_MaximumLength = strString.m_MaximumLength;
		m_Buffer = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR));
		memset(m_Buffer,0x00,(m_Length + 1) * sizeof(WCHAR));
		wcscpy(m_Buffer, strString.m_Buffer);
		m_TempBuff = NULL;
	}
}

CUnicodeString::~CUnicodeString()
{
	CString csTest = m_Buffer;
	if(m_Buffer) {
		free(m_Buffer);
		m_Buffer = NULL;
	}
	if(m_TempBuff) {
		free(m_TempBuff);
		m_TempBuff = NULL;
	}
}

const CUnicodeString& CUnicodeString::operator =(UNICODE_STRING* puString)
{
	m_MaximumLength = m_Length = (puString->Length / sizeof(WCHAR));
	m_Buffer = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR));
	memset(m_Buffer,0x00,(m_Length + 1) * sizeof(WCHAR));
	memcpy(m_Buffer, puString->Buffer, m_Length * sizeof(WCHAR));
	return *this;
}

const CUnicodeString& CUnicodeString::operator =(LPCWSTR pstrString)
{
	if(pstrString == NULL)
	{
		Empty();
		return *this;
	}

	ASSERT(IsValidString(pstrString));
	if(m_Buffer)
		free(m_Buffer);
	InitializeFromWideString(pstrString);
	return *this;
}

const CUnicodeString& CUnicodeString::operator =(LPCSTR pstrString)
{
	if(pstrString == NULL)
	{
		Empty();
		return *this;
	}
	ASSERT(IsValidString(pstrString));
	if(m_Buffer)
		free(m_Buffer);
	InitializeFromString(pstrString);
	return *this;
}

const CUnicodeString& CUnicodeString::operator =(WCHAR ch)
{
	if(m_Buffer)
		free(m_Buffer);
	WCHAR pstrString[] = L"\0\0";
	pstrString[0] = ch;
	InitializeFromWideString(pstrString);
	return *this;
}

const CUnicodeString& CUnicodeString::operator =(CHAR ch)
{
	if(m_Buffer)
		free(m_Buffer);
	CHAR pstrString[] = "\0\0";
	pstrString[0] = ch;
	InitializeFromString(pstrString);
	return *this;
}

const CUnicodeString& CUnicodeString::operator =(const CUnicodeString& strString)
{
	if(m_TempBuff)
		free(m_TempBuff);
	if(m_Buffer)
		free(m_Buffer);

	if(strString.IsEmpty())
	{
		m_Length = m_MaximumLength = 0;
		m_Buffer = NULL;
		m_TempBuff = NULL;
	}
	else
	{
		m_Length = strString.m_Length;
		m_MaximumLength = strString.m_MaximumLength;
		m_Buffer = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR));
		memset(m_Buffer,0x00,(m_Length + 1) * sizeof(WCHAR));
		wcscpy(m_Buffer, strString.m_Buffer);
		m_TempBuff = NULL;
	}
	return *this;
}

BOOL CUnicodeString::InitializeFromWideString(LPCWSTR pstrString)
{
	if(pstrString == NULL)
	{
		Empty();
		return TRUE;
	}

	ASSERT(IsValidString(pstrString));
	m_Length = wcslen(pstrString);
	m_MaximumLength = m_Length;
	m_Buffer = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR));
	memset(m_Buffer,0x00,(m_Length + 1) * sizeof(WCHAR));
	wcscpy(m_Buffer, pstrString);
	m_Buffer[m_Length] = L'\0';
	if(m_Buffer)
		return TRUE;
	return FALSE;
}

BOOL CUnicodeString::InitializeFromString(LPCSTR pstrString)
{
	if(pstrString == NULL)
	{
		Empty();
		return TRUE;
	}

	ASSERT(IsValidString(pstrString));
	m_Length = MultiByteToWideChar(CP_ACP, 0, pstrString, strlen(pstrString), NULL, 0);
	//	m_Length = strlen(pstrString);
	m_MaximumLength = m_Length;
	int bufferSize = (m_Length + 1) * sizeof(WCHAR);
	m_Buffer = (LPWSTR)malloc(bufferSize);
	memset(m_Buffer,0x00,bufferSize);
	int lengthWrite = MultiByteToWideChar(CP_ACP, 0, pstrString, strlen(pstrString), m_Buffer, bufferSize);
	m_Buffer[m_Length] = L'\0';
	if(m_Buffer)
		return TRUE;
	return FALSE;

	//	ASSERT(IsValidString(pstrString));
	//	m_Length = strlen(pstrString);
	//	m_MaximumLength = m_Length;
	//	m_Buffer = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR));
	//	memset(m_Buffer,0x00,(m_Length + 1) * sizeof(WCHAR));
	//	MultiByteToWideChar(CP_ACP, 0, pstrString, m_Length, m_Buffer, m_Length);
	//	m_Buffer[m_Length] = L'\0';
	//	if(m_Buffer)
	//		return TRUE;
	//	return FALSE;
}
/*
void CUnicodeString::operator += (const CUnicodeString& usString)
{
	m_Length += wcslen(usString);
	LPWSTR pstrNewString = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR));
	wcscpy(pstrNewString, m_Buffer);
	wcscat(pstrNewString, usString);
	free(m_Buffer);
	m_Buffer = pstrNewString;
	m_MaximumLength = m_Length;
}

void CUnicodeString::operator += (LPCWSTR pstrString)
{
	ASSERT(IsValidString(pstrString));
	m_Length += wcslen(pstrString);
	LPWSTR pstrNewString = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR));
	wcscpy(pstrNewString, m_Buffer);
	wcscat(pstrNewString, pstrString);
	free(m_Buffer);
	m_Buffer = pstrNewString;
	m_MaximumLength = m_Length;
}

void CUnicodeString::operator += (LPCSTR pstrString)
{
	ASSERT(IsValidString(pstrString));
	ULONG usNewLength = m_Length + strlen(pstrString);
	LPWSTR pstrNewString = (LPWSTR)malloc((usNewLength + 1) * sizeof(WCHAR));
	wcscpy(pstrNewString, m_Buffer);
	MultiByteToWideChar(CP_ACP, 0, pstrString, strlen(pstrString), &pstrNewString[m_Length], usNewLength - m_Length);
	free(m_Buffer);
	m_Buffer = pstrNewString;
	m_Length = usNewLength;
	m_MaximumLength = m_Length;
}

void CUnicodeString::operator += (WCHAR wch)
{
	m_Length++;
	LPWSTR pstrNewString = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR));
	wcscpy(pstrNewString, m_Buffer);
	pstrNewString[m_Length - 1] = wch;
	pstrNewString[m_Length] = L'\0';
	free(m_Buffer);
	m_Buffer = pstrNewString;
	m_MaximumLength = m_Length;
}

void CUnicodeString::operator += (CHAR ch)
{
	ULONG usNewLength = m_Length + 1;
	LPWSTR pstrNewString = (LPWSTR)malloc((usNewLength + 1) * sizeof(WCHAR));
	wcscpy(pstrNewString, m_Buffer);
	MultiByteToWideChar(CP_ACP, 0, &ch, 1, &pstrNewString[m_Length], usNewLength - m_Length);
	pstrNewString[usNewLength] = L'\0';
	free(m_Buffer);
	m_Buffer = pstrNewString;
	m_Length = usNewLength;
	m_MaximumLength = m_Length;
}
*/
//This was suggested by another developer for UnicodeString

void CUnicodeString::operator += (const CUnicodeString& usString) 
{ 
	if (usString.IsEmpty()) {
		return;
	}
	size_t nLen=wcslen(usString); 
	if( nLen==0 ) return; 

	m_Length += nLen; 
	LPWSTR pstrNewString = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR)); 
	if( m_Buffer ) 
	{ 
		wcscpy(pstrNewString, m_Buffer); 
		wcscat(pstrNewString, usString); 
		free(m_Buffer); 
	} 
	else 
	{ 
		wcscpy(pstrNewString, usString); 
	} 
	m_Buffer = pstrNewString; 
	m_MaximumLength = m_Length; 
} 

void CUnicodeString::operator += (LPCWSTR pstrString) 
{ 
	ASSERT(IsValidString(pstrString)); 
	if( pstrString==NULL ) return; 
	size_t nLen=wcslen(pstrString); 
	if( nLen==0 ) return; 

	m_Length += nLen; 
	LPWSTR pstrNewString = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR)); 
	if( m_Buffer ) 
	{ 
		wcscpy(pstrNewString, m_Buffer); 
		wcscat(pstrNewString, pstrString); 
		free(m_Buffer); 
	} 
	else 
	{ 
		wcscpy(pstrNewString, pstrString); 
	} 

	m_Buffer = pstrNewString; 
	m_MaximumLength = m_Length; 
} 

void CUnicodeString::operator += (LPCSTR pstrString) 
{ 
	ASSERT(IsValidString(pstrString)); 
	if( pstrString==NULL ) return; 
	//	size_t nLen=strlen(pstrString); 
	size_t nLen = MultiByteToWideChar(CP_ACP, 0, pstrString, strlen(pstrString), NULL, 0);
	if( nLen==0 ) return; 

	ULONG usNewLength = m_Length + nLen; 
	LPWSTR pstrNewString = (LPWSTR)malloc((usNewLength + 1) * sizeof(WCHAR)); 
	if( m_Buffer ) 
	{ 
		wcscpy(pstrNewString, m_Buffer); 
	} 
	MultiByteToWideChar(CP_ACP, 0, pstrString, strlen(pstrString), &pstrNewString[m_Length], nLen); 
	pstrNewString[usNewLength]=L'\0'; 
	if( m_Buffer ) 
	{ 
		free(m_Buffer); 
	} 
	m_Buffer = pstrNewString; 
	m_Length = usNewLength; 
	m_MaximumLength = m_Length; 
} 

void CUnicodeString::operator += (WCHAR wch) 
{ 
	m_Length++; 
	LPWSTR pstrNewString = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR)); 
	if( m_Buffer ) 
	{ 
		wcscpy(pstrNewString, m_Buffer); 
	} 
	pstrNewString[m_Length - 1] = wch; 
	pstrNewString[m_Length] = L'\0'; 
	if( m_Buffer ) 
	{ 
		free(m_Buffer); 
	} 
	m_Buffer = pstrNewString; 
	m_MaximumLength = m_Length; 
} 

void CUnicodeString::operator += (CHAR ch) 
{ 
	ULONG usNewLength = m_Length + 1; 
	LPWSTR pstrNewString = (LPWSTR)malloc((usNewLength + 1) * sizeof(WCHAR)); 
	if( m_Buffer ) 
	{ 
		wcscpy(pstrNewString, m_Buffer); 
	} 
	MultiByteToWideChar(CP_ACP, 0, &ch, 1, &pstrNewString[m_Length], usNewLength - m_Length); 
	pstrNewString[usNewLength] = L'\0'; 
	if( m_Buffer ) 
	{ 
		free(m_Buffer); 
	} 
	m_Buffer = pstrNewString; 
	m_Length = usNewLength; 
	m_MaximumLength = m_Length; 
}



// CUnicodeString::operator const LPCSTR ()

BOOL operator ==(const CUnicodeString& s1, const CUnicodeString& s2)
{
	if(wcslen(s1) != wcslen(s2))
		return FALSE;
	if(wcscmp(s1, s2))
		return FALSE;
	return TRUE;
}

BOOL operator ==(const CUnicodeString& s1, LPCWSTR s2)
{
	ASSERT(IsValidString(s2));
	if(wcslen(s1) != wcslen(s2))
		return FALSE;
	if(wcscmp(s1, s2))
		return FALSE;
	return TRUE;
}

BOOL operator ==(const CUnicodeString& s1, LPCSTR s2)
{
	ASSERT(IsValidString(s2));
	//	ULONG usLen = strlen(s2);
	ULONG usLen = MultiByteToWideChar(CP_ACP, 0, s2, strlen(s2), NULL, 0);
	if(usLen != wcslen(s1))
		return FALSE;
	BOOL bRet = TRUE;
	LPWSTR ws2 = (LPWSTR)malloc((usLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, s2, strlen(s2), ws2, usLen);
	ws2[usLen] = L'\0';
	if(wcscmp(s1, ws2))
		bRet = FALSE;
	free(ws2);
	return bRet;
}

BOOL operator ==(LPCWSTR s1, const CUnicodeString& s2)
{
	ASSERT(IsValidString(s1));
	if(wcslen(s1) != wcslen(s2))
		return FALSE;
	if(wcscmp(s1, s2))
		return FALSE;
	return TRUE;
}

BOOL operator ==(LPCSTR s1, const CUnicodeString& s2)
{
	ASSERT(IsValidString(s1));
	//	ULONG usLen = strlen(s1);
	ULONG usLen = MultiByteToWideChar(CP_ACP, 0, s1, strlen(s1), NULL, 0);
	if(usLen != wcslen(s2))
		return FALSE;
	BOOL bRet = TRUE;
	LPWSTR ws1 = (LPWSTR)malloc((usLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, s1, strlen(s1), ws1, usLen);
	ws1[usLen] = L'\0';
	if(wcscmp(ws1, s2))
		bRet = FALSE;
	free(ws1);
	return bRet;
}

BOOL operator !=(const CUnicodeString& s1, const CUnicodeString& s2)
{
	if(wcslen(s1) != wcslen(s2))
		return TRUE;
	if(!wcscmp(s1, s2))
		return FALSE;
	return TRUE;
}

BOOL operator !=(const CUnicodeString& s1, LPCWSTR s2)
{
	ASSERT(IsValidString(s2));
	if(wcslen(s1) != wcslen(s2))
		return TRUE;
	if(!wcscmp(s1, s2))
		return FALSE;
	return TRUE;
}

BOOL operator !=(const CUnicodeString& s1, LPCSTR s2)
{
	ASSERT(IsValidString(s2));
	//	ULONG usLen = strlen(s2);
	ULONG usLen = MultiByteToWideChar(CP_ACP, 0, s2, strlen(s2), NULL, 0);
	if(usLen != wcslen(s1))
		return TRUE;
	BOOL bRet = TRUE;
	LPWSTR ws2 = (LPWSTR)malloc((usLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, s2, strlen(s2), ws2, usLen);
	ws2[usLen] = L'\0';
	if(!wcscmp(s1, ws2))
		bRet = FALSE;
	free(ws2);
	return bRet;
}

BOOL operator !=(LPCWSTR s1, const CUnicodeString& s2)
{
	ASSERT(IsValidString(s1));
	if(wcslen(s1) != wcslen(s2))
		return TRUE;
	if(!wcscmp(s1, s2))
		return FALSE;
	return TRUE;
}

BOOL operator !=(LPCSTR s1, const CUnicodeString& s2)
{
	ASSERT(IsValidString(s1));
	//	ULONG usLen = strlen(s1);
	ULONG usLen = MultiByteToWideChar(CP_ACP, 0, s1, strlen(s1), NULL, 0);
	if(usLen != wcslen(s2))
		return TRUE;
	BOOL bRet = TRUE;
	LPWSTR ws1 = (LPWSTR)malloc((usLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, s1, strlen(s1), ws1, usLen);
	ws1[usLen] = L'\0';
	if(!wcscmp(ws1, s2))
		bRet = FALSE;
	free(ws1);
	return bRet;
}

void CUnicodeString::Format(LPCWSTR pstrFormat, ...)
{
	ASSERT(IsValidString(pstrFormat));
	va_list args;
	va_start(args, pstrFormat);

	int nBuf;
	WCHAR szBuffer[1024];

//	nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer), pstrFormat, args);
	nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), pstrFormat, args);

	if(m_Buffer)
		free(m_Buffer);
	m_Buffer = (LPWSTR)malloc((nBuf + 1) * sizeof(WCHAR));
	memset(m_Buffer,0x00,(nBuf + 1) * sizeof(WCHAR));
	m_Length = m_MaximumLength = nBuf;
	memcpy(m_Buffer, szBuffer, nBuf * sizeof(WCHAR));
	m_Buffer[m_Length] = L'\0';

	va_end(args);
}

void CUnicodeString::Format(LPCSTR pstrFormat, ...)
{
	ASSERT(IsValidString(pstrFormat));
	va_list args;
	va_start(args, pstrFormat);

	int nBuf;
	CHAR szBuffer[1024];

	nBuf = _vsnprintf(szBuffer, sizeof(szBuffer), pstrFormat, args);
	nBuf = MultiByteToWideChar(CP_ACP, 0, szBuffer, strlen(szBuffer), NULL, 0);
	if(m_Buffer)
		free(m_Buffer);
	m_Buffer = (LPWSTR)malloc((nBuf + 1) * sizeof(WCHAR));
	memset(m_Buffer,0x00,(nBuf + 1) * sizeof(WCHAR));
	m_Length = m_MaximumLength = nBuf;
	MultiByteToWideChar(CP_ACP, 0, szBuffer, strlen(szBuffer), m_Buffer, m_Length);
	m_Buffer[m_Length] = L'\0';

	va_end(args);
}

CUnicodeString operator+(const CUnicodeString& s1, const CUnicodeString& s2)
{
	CUnicodeString cus;
	cus = s1;
	cus += s2;
	return cus;
}

CUnicodeString operator+(const CUnicodeString& s, LPCSTR pstrString)
{
	ASSERT(IsValidString(pstrString));
	CUnicodeString cus;
	cus = s;
	cus += pstrString;
	return cus;
}

CUnicodeString operator+(LPCSTR pstrString, const CUnicodeString& s)
{
	ASSERT(IsValidString(pstrString));
	CUnicodeString cus;
	cus = pstrString;
	cus += s;
	return cus;
}

CUnicodeString operator+(const CUnicodeString& s, const char ch)
{
	CUnicodeString cus;
	cus = s;
	cus += ch;
	return cus;
}

CUnicodeString operator+(const char ch, const CUnicodeString& s)
{
	CUnicodeString cus;
	cus = ch;
	cus += s;
	return cus;
}

CUnicodeString operator+(const CUnicodeString& s, LPCWSTR pstrString)
{
	ASSERT(IsValidString(pstrString));
	CUnicodeString cus;
	cus = s;
	cus += pstrString;
	return cus;
}

CUnicodeString operator+(LPCWSTR pstrString, const CUnicodeString& s)
{
	ASSERT(IsValidString(pstrString));
	CUnicodeString cus;
	cus = pstrString;
	cus += s;
	return cus;
}

CUnicodeString operator+(const CUnicodeString& s, const WCHAR ch)
{
	CUnicodeString cus;
	cus = s;
	cus += ch;
	return cus;
}

CUnicodeString operator+(const WCHAR ch, const CUnicodeString& s)
{
	CUnicodeString cus;
	cus = ch;
	cus += s;
	return cus;
}

CUnicodeString CUnicodeString::Left(DWORD dwCount)
{
	return Mid(0, dwCount);
}

CUnicodeString CUnicodeString::Right(DWORD dwCount)
{
	return Mid(m_Length - dwCount, dwCount);
}

CUnicodeString CUnicodeString::Mid(DWORD dwStart, DWORD dwCount)
{
	ASSERT(dwStart < m_Length);
	ASSERT(dwStart+dwCount <= m_Length);
	CUnicodeString cus;
	cus.SetSize(dwCount);
	LPWSTR pucs = cus.GetBuffer();
	memcpy(pucs, &m_Buffer[dwStart], dwCount * sizeof(WCHAR));
	pucs[dwCount] = L'\0';
	return cus;
}

void CUnicodeString::SetSize(DWORD dwSize)
{
	m_Length = m_MaximumLength = (ULONG)dwSize;
	if(m_Buffer)
		free(m_Buffer);
	m_Buffer = (LPWSTR)malloc((dwSize + 1) * sizeof(WCHAR));
	memset(m_Buffer,0x00,(dwSize + 1) * sizeof(WCHAR));
}

LPWSTR CUnicodeString::GetBuffer(int nLen)
{
	return m_Buffer;
}

WCHAR CUnicodeString::operator[](DWORD dwPos) const
{
	if(dwPos >= (DWORD)m_Length)
		return L'\0';
	return m_Buffer[dwPos];
}

WCHAR CUnicodeString::GetAt(DWORD dwPos) const
{
	if(dwPos >= (DWORD)m_Length)
		return L'\0';
	return m_Buffer[dwPos];
}

BOOL CUnicodeString::IsEmpty() const
{
	return m_Length ? FALSE : TRUE;
}

void CUnicodeString::Empty()
{
	if(m_Buffer)
		free(m_Buffer);
	if(m_TempBuff)
		free(m_TempBuff);
	m_Buffer = NULL; m_TempBuff = NULL;
	m_Length = m_MaximumLength = 0;
}

void CUnicodeString::SetAt(DWORD dwPos, CHAR ch)
{
	ASSERT(dwPos < m_Length);
	CHAR c[] = "\0\0";
	c[0] = ch;
	MultiByteToWideChar(CP_ACP, 0, c, 1, &m_Buffer[dwPos], 1);
}

void CUnicodeString::SetAt(DWORD dwPos, WCHAR ch)
{
	ASSERT(dwPos < m_Length);
	m_Buffer[dwPos] = ch;
}

int CUnicodeString::Compare(LPCSTR lpsz)
{
	ASSERT(IsValidString(lpsz));
	//	DWORD dwszLen = strlen(lpsz);
	DWORD dwszLen = MultiByteToWideChar(CP_ACP, 0, lpsz, strlen(lpsz), NULL, 0);
	LPWSTR lpwsz = (LPWSTR)malloc((dwszLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, lpsz, strlen(lpsz), lpwsz, dwszLen);
	lpwsz[dwszLen] = L'\0';
	int iRet = wcscmp(m_Buffer, lpwsz);
	free(lpwsz);
	return iRet;
}

int CUnicodeString::Compare(LPCWSTR lpsz)
{
	ASSERT(IsValidString(lpsz));
	return wcscmp(m_Buffer, lpsz);
}

int CUnicodeString::CompareNoCase(LPCSTR lpsz)
{
	ASSERT(IsValidString(lpsz));
	//	DWORD dwszLen = strlen(lpsz);
	DWORD dwszLen = MultiByteToWideChar(CP_ACP, 0, lpsz, strlen(lpsz), NULL, 0);
	LPWSTR lpwsz = (LPWSTR)malloc((dwszLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, lpsz, strlen(lpsz), lpwsz, dwszLen);
	lpwsz[dwszLen] = L'\0';
	int iRet = wcsicmp(m_Buffer, lpwsz);
	free(lpwsz);
	return iRet;
}

int CUnicodeString::CompareNoCase(LPCWSTR lpsz)
{
	ASSERT(IsValidString(lpsz));
	return wcsicmp(m_Buffer, lpsz);
}

int CUnicodeString::Collate(LPCSTR lpsz)
{
	ASSERT(IsValidString(lpsz));
	//	DWORD dwszLen = strlen(lpsz);
	DWORD dwszLen = MultiByteToWideChar(CP_ACP, 0, lpsz, strlen(lpsz), NULL, 0);
	LPWSTR lpwsz = (LPWSTR)malloc((dwszLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, lpsz, strlen(lpsz), lpwsz, dwszLen);
	lpwsz[dwszLen] = L'\0';
	int iRet = wcscoll(m_Buffer, lpwsz);
	free(lpwsz);
	return iRet;
}

int CUnicodeString::Collate(LPCWSTR lpsz)
{
	ASSERT(IsValidString(lpsz));
	return wcscoll(m_Buffer, lpsz);
}

int CUnicodeString::CollateNoCase(LPCSTR lpsz)
{
	ASSERT(IsValidString(lpsz));
	//	DWORD dwszLen = strlen(lpsz);
	DWORD dwszLen = MultiByteToWideChar(CP_ACP, 0, lpsz, strlen(lpsz), NULL, 0);
	LPWSTR lpwsz = (LPWSTR)malloc((dwszLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, lpsz, strlen(lpsz), lpwsz, dwszLen);
	lpwsz[dwszLen] = L'\0';
	int iRet = wcsicoll(m_Buffer, lpwsz);
	free(lpwsz);
	return iRet;
}

int CUnicodeString::CollateNoCase(LPCWSTR lpsz)
{
	ASSERT(IsValidString(lpsz));
	return wcsicoll(m_Buffer, lpsz);
}

CUnicodeString CUnicodeString::SpanIncluding(LPCSTR lpszCharSet)
{
	ASSERT(IsValidString(lpszCharSet));
	WCHAR wc[1024];
	//	DWORD dwszLen = strlen(lpszCharSet);
	DWORD dwszLen = MultiByteToWideChar(CP_ACP, 0, lpszCharSet, strlen(lpszCharSet), NULL, 0);
	ASSERT(dwszLen < 1024);

	MultiByteToWideChar(CP_ACP, 0, lpszCharSet, strlen(lpszCharSet), wc, dwszLen);
	wc[dwszLen] = L'\0';
	return Left(wcsspn(m_Buffer, wc));
}

CUnicodeString CUnicodeString::SpanIncluding(LPCWSTR lpszCharSet)
{
	ASSERT(IsValidString(lpszCharSet));
	return Left(wcsspn(m_Buffer, lpszCharSet));
}

CUnicodeString CUnicodeString::SpanExcluding(LPCSTR lpszCharSet)
{
	ASSERT(IsValidString(lpszCharSet));
	WCHAR wc[1024];
	//	DWORD dwszLen = strlen(lpszCharSet);
	DWORD dwszLen = MultiByteToWideChar(CP_ACP, 0, lpszCharSet, strlen(lpszCharSet), NULL, 0);
	ASSERT(dwszLen < 1024);

	MultiByteToWideChar(CP_ACP, 0, lpszCharSet, strlen(lpszCharSet), wc, dwszLen);
	wc[dwszLen] = L'\0';

	return Left(wcscspn(m_Buffer, wc));
}

CUnicodeString CUnicodeString::SpanExcluding(LPCWSTR lpszCharSet)
{
	ASSERT(IsValidString(lpszCharSet));
	return Left(wcscspn(m_Buffer, lpszCharSet));
}

void CUnicodeString::MakeUpper()
{
	wcsupr(m_Buffer);
}

void CUnicodeString::MakeLower()
{
	wcslwr(m_Buffer);
}

void CUnicodeString::MakeReverse()
{
	wcsrev(m_Buffer);
}

int CUnicodeString::Remove(CHAR ch)
{
	CHAR c[] = "\0\0";
	c[0] = ch;
	WCHAR wc;
	MultiByteToWideChar(CP_ACP, 0, c, 1, &wc, 1);
	return Remove(wc);
}

int CUnicodeString::Remove(WCHAR ch)
{
	LPWSTR pstrSource = m_Buffer;
	LPWSTR pstrDest = m_Buffer;
	LPWSTR pstrEnd = m_Buffer + m_Length;

	while(pstrSource < pstrEnd)
	{
		if(*pstrSource != ch)
		{
			*pstrDest = *pstrSource;
			pstrDest = _wcsinc(pstrDest);
		}
		pstrSource = _wcsinc(pstrSource);
	}
	*pstrDest = '\0';
	int nCount = pstrSource - pstrDest;
	m_Length -= nCount;

	return nCount;
}

int CUnicodeString::Insert(int nIndex, LPCSTR pstr)
{
	ASSERT(IsValidString(pstr));
	//	DWORD dwszLen = strlen(pstr);
	DWORD dwszLen = MultiByteToWideChar(CP_ACP, 0, pstr, strlen(pstr), NULL, 0);
	LPWSTR lpwsz = (LPWSTR)malloc((dwszLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, pstr, strlen(pstr), lpwsz, dwszLen);
	lpwsz[dwszLen] = L'\0';
	int iRet = Insert(nIndex, lpwsz);
	free(lpwsz);
	return iRet;
}
int CUnicodeString::Insert(int nIndex, LPCWSTR pstr)
{
	ASSERT(IsValidString(pstr));
	ASSERT(nIndex < m_Length);
	ASSERT(m_Buffer);
	DWORD dwszLen = wcslen(pstr);
	int iNewLength = (m_Length + dwszLen + 1) * sizeof(WCHAR);
	LPWSTR pstrNew = (LPWSTR)malloc((iNewLength + 1 ) * sizeof(WCHAR));
	memset(pstrNew,0x00,sizeof(WCHAR) * (iNewLength +1));
	wcsncpy(pstrNew, m_Buffer, nIndex);
	wcscat(pstrNew, pstr);
	wcscat(pstrNew, &m_Buffer[nIndex]);
	free(m_Buffer);
	m_Buffer = NULL;
	m_Buffer = pstrNew;
	m_Length = m_MaximumLength = (iNewLength / 2) - 1;
	return m_Length;
}

int CUnicodeString::Insert(int nIndex, CHAR ch)
{
	CHAR c[] = "\0\0";
	c[0] = ch;
	WCHAR wc[] = L"\0\0";
	MultiByteToWideChar(CP_ACP, 0, c, 1, wc, 1);
	return Insert(nIndex, wc);
}

int CUnicodeString::Insert(int nIndex, WCHAR ch)
{
	WCHAR wc[] = L"\0\0";
	wc[0] = ch;
	return Insert(nIndex, wc);
}

int CUnicodeString::Delete(int nIndex, int nCount)
{
	ASSERT(m_Buffer);
	ASSERT(nIndex + nCount < m_Length);
	m_Length -= nCount;
	m_MaximumLength = m_Length;
	LPWSTR pstrNew = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR));
	memset(pstrNew,0x00,sizeof(WCHAR) * (m_Length +1));
	wcsncpy(pstrNew, m_Buffer, nIndex);
	wcscat(pstrNew, &m_Buffer[nIndex + nCount]);
	free(m_Buffer);
	m_Buffer = pstrNew;
	return m_Length;
}

void CUnicodeString::FormatV(LPCSTR lpszFormat, va_list argList)
{
	ASSERT(IsValidString(lpszFormat));

	int nBuf;
	CHAR szBuffer[1024];

	nBuf = _vsnprintf(szBuffer, sizeof(szBuffer), lpszFormat, argList);
	nBuf = MultiByteToWideChar(CP_ACP, 0, szBuffer, strlen(szBuffer), NULL, 0);
	if(m_Buffer)
		free(m_Buffer);
	m_Buffer = (LPWSTR)malloc((nBuf + 1) * sizeof(WCHAR));
	memset(m_Buffer,0x00,(nBuf + 1) * sizeof(WCHAR));
	m_Length = m_MaximumLength = nBuf;

	MultiByteToWideChar(CP_ACP, 0, szBuffer, strlen(szBuffer), m_Buffer, m_Length);

	m_Buffer[m_Length] = L'\0';
}

void CUnicodeString::FormatV(LPCWSTR lpszFormat, va_list argList)
{
	ASSERT(IsValidString(lpszFormat));

	int nBuf;
	WCHAR szBuffer[1024];

	nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer), lpszFormat, argList);
//	nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), pstrFormat, args);

	if(m_Buffer)
		free(m_Buffer);
	m_Buffer = (LPWSTR)malloc((nBuf + 1) * sizeof(WCHAR));
	memset(m_Buffer,0x00,(nBuf + 1) * sizeof(WCHAR));
	m_Length = m_MaximumLength = nBuf;
	memcpy(m_Buffer, szBuffer, nBuf * sizeof(WCHAR));
	m_Buffer[m_Length] = L'\0';
}

void CUnicodeString::TrimLeft()
{
	LPCWSTR lpsz = m_Buffer;

	while (iswspace(*lpsz))
		lpsz = _wcsinc(lpsz);

	if(lpsz != m_Buffer)
	{
		// fix up data and length
		int nDataLength = m_Length - (lpsz - m_Buffer);
		memmove(m_Buffer, lpsz, (nDataLength+1)*sizeof(WCHAR));
		m_Length = nDataLength;
	}
}

void CUnicodeString::TrimLeft(LPCSTR lpszTargets)
{
	ASSERT(IsValidString(lpszTargets));
	//	DWORD dwszLen = strlen(lpszTargets);
	DWORD dwszLen = MultiByteToWideChar(CP_ACP, 0, lpszTargets, strlen(lpszTargets), NULL, 0);
	LPWSTR lpwsz = (LPWSTR)malloc((dwszLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, lpszTargets, strlen(lpszTargets), lpwsz, dwszLen);
	lpwsz[dwszLen] = L'\0';
	TrimLeft(lpwsz);
	free(lpwsz);
}

void CUnicodeString::TrimLeft(LPCWSTR lpszTargets)
{
	ASSERT(IsValidString(lpszTargets));
	LPCWSTR lpsz = m_Buffer;

	while(*lpsz != '\0')
	{
		if(wcschr(lpszTargets, *lpsz) == NULL)
			break;
		lpsz = _wcsinc(lpsz);
	}

	if(lpsz != m_Buffer)
	{
		// fix up data and length
		int nDataLength = m_Length - (lpsz - m_Buffer);
		memmove(m_Buffer, lpsz, (nDataLength+1)*sizeof(WCHAR));
		m_Length = nDataLength;
	}
}

void CUnicodeString::TrimLeft(CHAR chTarget)
{
	CHAR c[] = "\0\0";
	c[0] = chTarget;
	WCHAR wc[] = L"\0\0";;
	MultiByteToWideChar(CP_ACP, 0, c, 1, wc, 1);
	TrimLeft(wc);
}

void CUnicodeString::TrimLeft(WCHAR chTarget)
{
	WCHAR c[] = L"\0\0";
	c[0] = chTarget;
	TrimLeft(c);
}

void CUnicodeString::TrimRight()
{
	LPWSTR lpsz = m_Buffer;
	LPWSTR lpszLast = NULL;

	while(*lpsz != L'\0')
	{
		if(iswspace(*lpsz))
		{
			if(lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = _wcsinc(lpsz);
	}

	if(lpszLast != NULL)
	{
		// truncate at trailing space start
		*lpszLast = L'\0';
		m_Length = lpszLast - m_Buffer;
	}
}

void CUnicodeString::TrimRight(LPCSTR lpszTargets)
{
	ASSERT(IsValidString(lpszTargets));
	//	DWORD dwszLen = strlen(lpszTargets);
	DWORD dwszLen = MultiByteToWideChar(CP_ACP, 0, lpszTargets, strlen(lpszTargets), NULL, 0);
	LPWSTR lpwsz = (LPWSTR)malloc((dwszLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, lpszTargets, strlen(lpszTargets), lpwsz, dwszLen);
	lpwsz[dwszLen] = L'\0';
	TrimRight(lpwsz);
	free(lpwsz);
}

void CUnicodeString::TrimRight(LPCWSTR lpszTargets)
{
	ASSERT(IsValidString(lpszTargets));
	LPWSTR lpsz = m_Buffer;
	LPWSTR lpszLast = NULL;

	while(*lpsz != '\0')
	{
		if(wcschr(lpszTargets, *lpsz) != NULL)
		{
			if(lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = _wcsinc(lpsz);
	}

	if(lpszLast != NULL)
	{
		// truncate at left-most matching character
		*lpszLast = '\0';
		m_Length = lpszLast - m_Buffer;
	}
}

void CUnicodeString::TrimRight(CHAR chTarget)
{
	CHAR c[] = "\0\0";
	c[0] = chTarget;
	WCHAR wc[] = L"\0\0";;
	MultiByteToWideChar(CP_ACP, 0, c, 1, wc, 1);
	TrimRight(wc);
}

void CUnicodeString::TrimRight(WCHAR chTarget)
{
	WCHAR c[] = L"\0\0";
	c[0] = chTarget;
	TrimRight(c);
}

int CUnicodeString::Find(LPCSTR lpszSub, int nStart) const
{
	ASSERT(IsValidString(lpszSub));
	//	DWORD dwszLen = strlen(lpszSub);
	DWORD dwszLen = MultiByteToWideChar(CP_ACP, 0, lpszSub, strlen(lpszSub), NULL, 0);
	LPWSTR lpwsz = (LPWSTR)malloc((dwszLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, lpszSub, strlen(lpszSub), lpwsz, dwszLen);
	lpwsz[dwszLen] = L'\0';
	int iRet = Find(lpwsz, nStart);
	free(lpwsz);
	return iRet;
}

int CUnicodeString::Find(LPCWSTR lpszSub, int nStart) const
{
	ASSERT(IsValidString(lpszSub));

	int nLength = m_Length;
	if(nStart > nLength)
		return -1;

	// find first matching substring
	LPWSTR lpsz = wcsstr(m_Buffer + nStart, lpszSub);

	// return -1 for not found, distance from beginning otherwise
	return(lpsz == NULL) ? -1 : (int)(lpsz - m_Buffer);
}

int CUnicodeString::Find(CHAR ch, int nStart) const
{
	CHAR c[] = "\0\0";
	c[0] = ch;
	WCHAR wc[] = L"\0\0";;
	MultiByteToWideChar(CP_ACP, 0, c, 1, wc, 1);
	return Find(wc, nStart);
}

int CUnicodeString::Find(WCHAR ch, int nStart) const
{
	WCHAR c[] = L"\0\0";
	c[0] = ch;
	return Find(c);
}

int CUnicodeString::ReverseFind(CHAR ch) const
{
	CHAR c[] = "\0\0";
	c[0] = ch;
	WCHAR wc[] = L"\0\0";;
	MultiByteToWideChar(CP_ACP, 0, c, 1, wc, 1);
	return ReverseFind(wc[0]);
}

int CUnicodeString::ReverseFind(WCHAR ch) const
{
	// find last single character
	LPWSTR lpsz = wcsrchr(m_Buffer, ch);

	// return -1 if not found, distance from beginning otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_Buffer);
}

int CUnicodeString::FindOneOf(LPCSTR lpszCharSet) const
{
	ASSERT(IsValidString(lpszCharSet));
	//	DWORD dwszLen = strlen(lpszCharSet);
	DWORD dwszLen = MultiByteToWideChar(CP_ACP, 0, lpszCharSet, strlen(lpszCharSet), NULL, 0);
	LPWSTR lpwsz = (LPWSTR)malloc((dwszLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, lpszCharSet, strlen(lpszCharSet), lpwsz, dwszLen);
	lpwsz[dwszLen] = L'\0';
	int iRet = FindOneOf(lpwsz);
	free(lpwsz);
	return iRet;
}

int CUnicodeString::FindOneOf(LPCWSTR lpszCharSet) const
{
	ASSERT(IsValidString(lpszCharSet));
	LPWSTR lpsz = wcspbrk(m_Buffer, lpszCharSet);
	return (lpsz == NULL) ? - 1 : (int)(lpsz - m_Buffer);
}

void CUnicodeString::FormatMessage(UINT nFormatID, ...)
{
	// get format string from string table
	CUnicodeString strFormat;
	VERIFY(strFormat.LoadString(nFormatID) != 0);

	// format message into temporary buffer lpszTemp
	va_list argList;
	va_start(argList, nFormatID);
	LPWSTR lpszTemp;
	if (::FormatMessageW(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER, strFormat, 0, 0, (LPWSTR)&lpszTemp, 0, &argList) == 0 || lpszTemp == NULL)
		return;

	int iLen = wcslen(lpszTemp);
	m_Length = iLen;
	m_Buffer = (LPWSTR)malloc((iLen + 1) * sizeof(WCHAR));
	memset(m_Buffer,0x00,(iLen + 1) * sizeof(WCHAR));
	wcscpy(m_Buffer, lpszTemp);
	LocalFree(lpszTemp);
	va_end(argList);
}

void CUnicodeString::FormatMessage(LPCSTR lpszFormat, ...)
{
	if(m_Buffer)
		free(m_Buffer);

	va_list argList;
	va_start(argList, lpszFormat);
	LPSTR lpszTemp;

	if(::FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER, lpszFormat, 0, 0, (LPSTR)&lpszTemp, 0, &argList) == 0 || lpszTemp == NULL)
		return;

	//	int iLen = strlen(lpszTemp);
	int iLen = MultiByteToWideChar(CP_ACP, 0, lpszTemp, strlen(lpszTemp), NULL, 0);
	m_Length = iLen;
	m_Buffer = (LPWSTR)malloc((iLen + 1) * sizeof(WCHAR));
	memset(m_Buffer,0x00,(iLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, lpszTemp, strlen(lpszTemp), m_Buffer, m_Length);
	m_Buffer[m_Length] = L'\0';
	LocalFree(lpszTemp);
	va_end(argList);
}

void CUnicodeString::FormatMessage(LPCWSTR lpszFormat, ...)
{
	if(m_Buffer)
		free(m_Buffer);

	va_list argList;
	va_start(argList, lpszFormat);
	LPWSTR lpszTemp;

	if(::FormatMessageW(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER, lpszFormat, 0, 0, (LPWSTR)&lpszTemp, 0, &argList) == 0 ||	lpszTemp == NULL)
		return;

	int iLen = wcslen(lpszTemp);
	m_Length = iLen;
	m_Buffer = (LPWSTR)malloc((iLen + 1) * sizeof(WCHAR));
	memset(m_Buffer,0x00,(iLen + 1) * sizeof(WCHAR));
	wcscpy(m_Buffer, lpszTemp);
	LocalFree(lpszTemp);
	va_end(argList);
}

#ifndef _NO_MFC_SUPPORT
BOOL CUnicodeString::LoadString(UINT nID)
{
	int nMaxBuf = 1024;
	LPWSTR lpszBuf = (LPWSTR)malloc((nMaxBuf + 1) * sizeof(WCHAR));
Load:
	int nLen = ::LoadStringW(AfxGetResourceHandle(), nID, lpszBuf, nMaxBuf);
	if(nMaxBuf - nLen <= 1)
	{
		nMaxBuf += 1024;
		lpszBuf = (LPWSTR)realloc(lpszBuf, nMaxBuf);
		goto Load;
	}
	if(m_Length && m_Buffer)
		free(m_Buffer);
	m_Length = wcslen(lpszBuf);
	m_Buffer = (LPWSTR)malloc((m_Length + 1) * sizeof(WCHAR));
	memset(m_Buffer,0x00,(m_Length + 1) * sizeof(WCHAR));
	wcscpy(m_Buffer, lpszBuf);
	free(lpszBuf);
	return FALSE;
}
#endif

#ifndef _NO_BSTR_SUPPORT
BSTR CUnicodeString::AllocSysString() const
{
	return ::SysAllocStringLen(m_Buffer, m_Length);
}

BSTR CUnicodeString::SetSysString(BSTR *pbstr) const
{
	ASSERT(!IsBadWritePtr(pbstr, sizeof(BSTR)));
	if(!::SysReAllocStringLen(pbstr, m_Buffer, m_Length))
		ASSERT(FALSE);
	return *pbstr;
}
#endif

DWORD CUnicodeString::GetLength()
{
	return m_Length;
};

CUnicodeString::operator const LPWSTR () const
{
	return m_Buffer;
}

CUnicodeString::operator const LPCSTR ()
{
	if(m_TempBuff)
		free(m_TempBuff);

//	m_TempBuff = (LPSTR)malloc((m_Length + 2) * sizeof(WCHAR));
//	memset(m_TempBuff,0x00,(m_Length + 2) * sizeof(WCHAR));
	int nLen = WideCharToMultiByte(CP_ACP, 0, m_Buffer, m_Length, NULL, 0, NULL, NULL);
	m_TempBuff = (LPSTR)malloc((nLen + 1) * sizeof(CHAR));
	memset(m_TempBuff,0x00,(nLen + 1) * sizeof(CHAR));
	WideCharToMultiByte(CP_ACP, 0, m_Buffer, m_Length, m_TempBuff, nLen, NULL, NULL);
//	WideCharToMultiByte(CP_ACP, 0, m_Buffer, m_Length, m_TempBuff, (m_Length + 2) * sizeof(WCHAR), NULL, NULL);
	m_TempBuff[nLen] = '\0';
//	m_TempBuff[(m_Length + 2) * sizeof(WCHAR)-1] = '\0';
	return m_TempBuff;
}

CUnicodeString::operator PUNICODE_STRING ()
{
	m_usBuffer.Length = m_usBuffer.MaximumLength = (unsigned short)m_Length * sizeof(WCHAR);
	m_usBuffer.Buffer = m_Buffer;
	return &m_usBuffer;
}
