///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1999/2000 Alexander Keck
//
//  $Workfile: UnicodeString.h $
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

#if !defined(AFX_UNICODESTRING_H__1C328430_CFAB_11D2_A733_0000E83A16D9__INCLUDED_)
#define AFX_UNICODESTRING_H__1C328430_CFAB_11D2_A733_0000E83A16D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <NTSecApi.h>
/*
//#ifndef UNICODE_STRING
#ifndef _NTSECAPI_
typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
#endif
*/
BOOL IsValidString(LPCSTR lpsz, int nLength = -1);
BOOL IsValidString(LPCWSTR lpsz, int nLength = -1);

class CUnicodeString
{
protected:

	ULONG m_Length;
	ULONG m_MaximumLength;
	LPWSTR  m_Buffer;
	LPSTR   m_TempBuff;
	UNICODE_STRING m_usBuffer;
public:
#ifndef _NO_BSTR_SUPPORT
	BSTR SetSysString(BSTR* pbstr) const;
	BSTR AllocSysString() const;
#endif
#ifndef _NO_MFC_SUPPORT
	BOOL LoadString(UINT nID);
#endif
	void FormatMessage(UINT nFormatID, ...);
	void FormatMessage(LPCWSTR lpszFormat, ...);
	void FormatMessage(LPCSTR lpszFormat, ...);
	int FindOneOf(LPCSTR lpszCharSet) const;
	int FindOneOf(LPCWSTR lpszCharSet) const;
	int ReverseFind(CHAR ch) const;
	int ReverseFind(WCHAR ch) const;
	int Find(CHAR ch, int nStart = 0) const;
	int Find(WCHAR ch, int nStart = 0) const;
	int Find(LPCSTR lpszSub, int nStart = 0) const;
	int Find(LPCWSTR lpszSub, int nStart = 0) const;
	void TrimRight(LPCSTR lpszTargets);
	void TrimRight(LPCWSTR lpszTargets);
	void TrimRight(CHAR chTarget);
	void TrimRight(WCHAR chTarget);
	void TrimRight();
	void TrimLeft(LPCSTR lpszTargets);
	void TrimLeft(LPCWSTR lpszTargets);
	void TrimLeft(CHAR chTarget);
	void TrimLeft(WCHAR chTarget);
	void TrimLeft();
	void FormatV(LPCSTR lpszFormat, va_list argList);
	void FormatV(LPCWSTR lpszFormat, va_list argList);
	int Delete(int nIndex, int nCount = 1);
	int Insert(int nIndex, LPCSTR pstr);
	int Insert(int nIndex, LPCWSTR pstr);
	int Insert(int nIndex, CHAR ch);
	int Insert(int nIndex, WCHAR ch);
	int Remove(CHAR ch);
	int Remove(WCHAR ch);
	void MakeReverse();
	void MakeLower();
	void MakeUpper();
	CUnicodeString SpanExcluding(LPCSTR lpszCharSet);
	CUnicodeString SpanExcluding(LPCWSTR lpszCharSet);
	CUnicodeString SpanIncluding(LPCSTR lpszCharSet);
	CUnicodeString SpanIncluding(LPCWSTR lpszCharSet);
	int CollateNoCase(LPCSTR lpsz);
	int CollateNoCase(LPCWSTR lpsz);
	int Collate(LPCSTR lpsz);
	int Collate(LPCWSTR lpsz);
	int CompareNoCase(LPCSTR lpsz);
	int CompareNoCase(LPCWSTR lpsz);
	int Compare(LPCSTR lpsz);
	int Compare(LPCWSTR lpsz);
	void SetAt(DWORD dwPos, CHAR ch);
	void SetAt(DWORD dwPos, WCHAR ch);
	void Empty();
	BOOL IsEmpty() const;
	LPWSTR GetBuffer(int nLen = 0);
	void SetSize(DWORD dwSize);
	CUnicodeString Mid(DWORD dwStart, DWORD dwCount);
	CUnicodeString Right(DWORD dwCount);
	CUnicodeString Left(DWORD dwCount);
	void Format(LPCWSTR pstrFormat, ...);
	void Format(LPCSTR pstrFormat, ...);
	DWORD GetLength();
	CUnicodeString();
	CUnicodeString(UNICODE_STRING* puString);
	CUnicodeString(LPCWSTR pstrString);
	CUnicodeString(LPCSTR pstrString);
	CUnicodeString(const CUnicodeString& strString);
	const CUnicodeString& operator =(const CUnicodeString& strString);
	const CUnicodeString& operator =(UNICODE_STRING* puString);
	const CUnicodeString& operator =(LPCWSTR pstrString);
	const CUnicodeString& operator =(LPCSTR pstrString);
	const CUnicodeString& operator =(WCHAR ch);
	const CUnicodeString& operator =(CHAR ch);
	void operator += (const CUnicodeString& usString);
	void operator += (LPCWSTR pstrString);
	void operator += (LPCSTR pstrString);
	void operator += (WCHAR wch);
	void operator += (CHAR ch);
	operator const LPWSTR () const;
	operator const LPCSTR ();
	operator PUNICODE_STRING ();
	friend CUnicodeString operator+(const CUnicodeString& s1, const CUnicodeString& s2);
	friend CUnicodeString operator+(const CUnicodeString& s, LPCSTR pstrString);
	friend CUnicodeString operator+(LPCSTR pstrString, const CUnicodeString& s);
	friend CUnicodeString operator+(const CUnicodeString& s, const char ch);
	friend CUnicodeString operator+(const char ch, const CUnicodeString& s);
	friend CUnicodeString operator+(const CUnicodeString& s, LPCWSTR pstrString);
	friend CUnicodeString operator+(LPCWSTR pstrString, const CUnicodeString& s);
	friend CUnicodeString operator+(const CUnicodeString& s, const WCHAR ch);
	friend CUnicodeString operator+(const WCHAR ch, const CUnicodeString& s);
	WCHAR operator[](DWORD dwPos) const;
	WCHAR GetAt(DWORD dwPos) const;
	virtual ~CUnicodeString();

protected:
	BOOL InitializeFromWideString(LPCWSTR pstrString);
	BOOL InitializeFromString(LPCSTR pstrString);
};

BOOL operator ==(const CUnicodeString& s1, const CUnicodeString& s2);
BOOL operator ==(const CUnicodeString& s1, LPCWSTR s2);
BOOL operator ==(const CUnicodeString& s1, LPCSTR s2);
BOOL operator ==(LPCWSTR s1, const CUnicodeString& s2);
BOOL operator ==(LPCSTR s1, const CUnicodeString& s2);
BOOL operator !=(const CUnicodeString& s1, const CUnicodeString& s2);
BOOL operator !=(const CUnicodeString& s1, LPCWSTR s2);
BOOL operator !=(const CUnicodeString& s1, LPCSTR s2);
BOOL operator !=(LPCWSTR s1, const CUnicodeString& s2);
BOOL operator !=(LPCSTR s1, const CUnicodeString& s2);

#ifndef _NO_MFC_SUPPORT
//typedef CArray<CUnicodeString, CUnicodeString> CUnicodeStringArray;
#endif

#endif // !defined(AFX_UNICODESTRING_H__1C328430_CFAB_11D2_A733_0000E83A16D9__INCLUDED_)

