///////////////////////////////////////////////////////////////////
//                 Ini.cpp
//
// "CIni" is a simple API wrap class used for ini file access.
// The purpose of this class is to make ini file access more
// convenient than direct API calls.
//	
// This file is distributed "as is" and without any expressed or implied
// warranties. The author holds no responsibilities for any possible damages
// or loss of data that are caused by use of this file. The user must assume
// the entire risk of using this file.
//
// 7/08/2002    Bin Liu
//
// Update history:
//
//  7/08/2002 -- Initial release.
//  7/14/2002 -- Added "IncreaseInt" and "AppendString"
//  9/02/2002 -- Added "removeProfileSection" and "RemoveProfileEntry"
//  2/09/2003 -- The class has been made unicode-compliant
// 11/04/2003 -- Integrated MFC support, added in new member functions
//               for accessing arrays.
// 11/08/2003 -- Fixed "GetString" and "GetPathName" method, changed parameter
//               from "LPSTR" to "LPTSTR"
// 11/10/2003 -- Renamed method "GetKeys" to "GetKeyLines",
//               Added method "GetKeyNames"
//               Added parameter "bTrimString" to method "GetArray"
// 11/14/2003 -- Use "__AFXWIN_H__" instead of "_AFXDLL" to determine MFC presence
//               Removed length limit on "m_pszPathName"
//               Removed "GetStruct" and "WriteStruct"
//               Added "GetDataBlock", "WriteDataBlock", "AppendDataBlock"
//               Added "GetChar" and "WriteChar"
//
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Cini Class Implementation
/////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h" // include if you got "fatal error C1010: unexpected end of file..."
#include "Ini.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define DEF_PROFILE_NUM_LEN		64 // numeric string length, could be quite long for binary format
#define DEF_PROFILE_THRESHOLD	512 // temporary string length
#define DEF_PROFILE_DELIMITER	_T(",") // default string delimiter
#define DEF_PROFILE_TESTSTRING	_T("{63788286-AE30-4D6B-95DF-3B451C1C79F9}") // Uuid for internal use

// struct used to be passed to __KeyPairProc as a LPVOID parameter
struct STR_LIMIT
{
	LPTSTR lpTarget;
	DWORD dwRemain;
	DWORD dwTotalCopied;
};

/////////////////////////////////////////////////////////////////////////////////
// Constructors & Destructor
/////////////////////////////////////////////////////////////////////////////////
CIni::CIni()
{
	m_pszPathName = NULL;
}

CIni::CIni(LPCTSTR lpPathName)
{
	m_pszPathName = NULL;
	SetPathName(lpPathName);
}

CIni::~CIni()
{
	if (m_pszPathName != NULL)
	{	
		//delete [] m_pszPathName;
		free(m_pszPathName);


		m_pszPathName = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////
// Ini File Path Access
/////////////////////////////////////////////////////////////////////////////////

// Assign ini file path name
void CIni::SetPathName(LPCTSTR lpPathName)
{
	if (lpPathName == NULL)
	{
		if (m_pszPathName != NULL)
			*m_pszPathName = _T('\0');
	}
	else
	{
		if (m_pszPathName != NULL)
			//delete [] m_pszPathName;
			free(m_pszPathName);

		m_pszPathName = _tcsdup(lpPathName);
	}
}

// Retrieve ini file path name
DWORD CIni::GetPathName(LPTSTR lpBuffer, DWORD dwBufSize) const
{
	*lpBuffer = _T('\0');
	DWORD dwLen = 0;
	if (lpBuffer != NULL)
	{
		_tcsncpy(lpBuffer, m_pszPathName, dwBufSize);
		dwLen = _tcslen(lpBuffer);
	}
	else
	{
		// just calculate the required buffer size
		dwLen = _tcslen(m_pszPathName);
	}
	return dwLen;
}

#ifdef __AFXWIN_H__
CString CIni::GetPathName() const
{
	return CString(m_pszPathName);
}
#endif

/////////////////////////////////////////////////////////////////////////////////
// Raw String Access
/////////////////////////////////////////////////////////////////////////////////

// Get a profile string value, if the buffer size is not large enough, the result
// may be truncated.
DWORD CIni::GetString(LPCTSTR lpSection, LPCTSTR lpKey, LPTSTR lpBuffer, DWORD dwBufSize, LPCTSTR lpDefault) const
{
	if (lpBuffer != NULL)
		*lpBuffer = _T('\0');

	LPTSTR psz = __GetStringDynamic(lpSection, lpKey, lpDefault);
	DWORD dwLen = _tcslen(psz);

	if (lpBuffer != NULL)
	{
		_tcsncpy(lpBuffer, psz, dwBufSize);
		dwLen = min(dwLen, dwBufSize);
	}

	delete [] psz;
	return dwLen;
}

#ifdef __AFXWIN_H__
CString CIni::GetString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpDefault) const
{
	LPTSTR psz = __GetStringDynamic(lpSection, lpKey, lpDefault);
	CString str(psz);
	delete [] psz;
	return str;
}
#endif

// Write a string value to the ini file
BOOL CIni::WriteString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpValue) const
{
	if (lpSection == NULL || lpKey == NULL)
		return FALSE;

	return ::WritePrivateProfileString(lpSection, lpKey, lpValue == NULL ? _T("") : lpValue, m_pszPathName);
}

// Read a string value from the ini file, append another string after it and then write it
// back to the ini file
BOOL CIni::AppendString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpString) const
{
	if (lpString == NULL)
		return FALSE;

	TCHAR* psz = __GetStringDynamic(lpSection, lpKey);
	TCHAR* pNewString = new TCHAR[_tcslen(psz) + _tcslen(lpString) + 1];
	_stprintf(pNewString, _T("%s%s"), psz, lpString);
	const BOOL RES = WriteString(lpSection, lpKey, pNewString);
	delete [] pNewString;
	delete [] psz;
	return RES;
}

/////////////////////////////////////////////////////////////////////////////////
// Ini File String Array Access
/////////////////////////////////////////////////////////////////////////////////

// Get an array of string
DWORD CIni::GetArray(LPCTSTR lpSection, LPCTSTR lpKey, LPTSTR lpBuffer, DWORD dwBufSize, LPCTSTR lpDelimiter, BOOL bTrimString) const
{
	if (lpBuffer != NULL)
		*lpBuffer = _T('\0');

	if (lpSection == NULL || lpKey == NULL)
		return 0;	

	LPTSTR psz = __GetStringDynamic(lpSection, lpKey);
	
	DWORD dwCopied = 0;

	if (*psz != _T('\0'))
	{
		if (lpBuffer == NULL)
		{
			// just calculate the required buffer size
			const DWORD MAX_LEN = _tcslen(psz) + 2;
			LPTSTR p = new TCHAR[MAX_LEN + 1];
			dwCopied = __StringSplit(psz, p, MAX_LEN, lpDelimiter, bTrimString);
			delete [] p;
		}
		else
		{
			dwCopied = __StringSplit(psz, lpBuffer, dwBufSize, lpDelimiter, bTrimString);
		}
	}		

	delete [] psz;
	return dwCopied;
}

#ifdef __AFXWIN_H__
void CIni::GetArray(LPCTSTR lpSection, LPCTSTR lpKey, CStringArray *pArray, LPCTSTR lpDelimiter, BOOL bTrimString) const
{
	if (pArray != NULL)
		pArray->RemoveAll();

	const DWORD LEN = GetArray(lpSection, lpKey, NULL, 0, lpDelimiter);
	if (LEN == 0)
		return;

	LPTSTR psz = new TCHAR[LEN + 3];
	GetArray(lpSection, lpKey, psz, LEN + 2, lpDelimiter);
	ParseDNTString(psz, __SubStrAdd, (LPVOID)pArray);
	delete [] psz;
}
#endif

#ifdef __AFXWIN_H__
BOOL CIni::WriteArray(LPCTSTR lpSection, LPCTSTR lpKey, const CStringArray *pArray, int nWriteCount, LPCTSTR lpDelimiter) const
{
	if (pArray == NULL)
		return FALSE;

	if (nWriteCount < 0)
		nWriteCount = pArray->GetSize();
	else
		nWriteCount = min(nWriteCount, pArray->GetSize());

	const CString DELIMITER = (lpDelimiter == NULL || *lpDelimiter == _T('\0')) ? _T(",") : lpDelimiter;
	CString sLine;
	for (int i = 0; i < nWriteCount; i++)
	{
		sLine += pArray->GetAt(i);
		if (i != nWriteCount - 1)
			sLine += DELIMITER;
	}
	return WriteString(lpSection, lpKey, sLine);
}
#endif

/////////////////////////////////////////////////////////////////////////////////
// Primitive Data Type Access
/////////////////////////////////////////////////////////////////////////////////

// Get a signed integral value
int CIni::GetInt(LPCTSTR lpSection, LPCTSTR lpKey, int nDefault, int nBase) const
{
	TCHAR sz[DEF_PROFILE_NUM_LEN + 1] = _T("");
	GetString(lpSection, lpKey, sz, DEF_PROFILE_NUM_LEN);
	return *sz == _T('\0') ? nDefault : int(_tcstoul(sz, NULL, __ValidateBase(nBase)));
}

// Get an unsigned integral value
UINT CIni::GetUInt(LPCTSTR lpSection, LPCTSTR lpKey, UINT nDefault, int nBase) const
{
	TCHAR sz[DEF_PROFILE_NUM_LEN + 1] = _T("");
	GetString(lpSection, lpKey, sz, DEF_PROFILE_NUM_LEN);
	return *sz == _T('\0') ? nDefault : UINT(_tcstoul(sz, NULL, __ValidateBase(nBase)));
}

// Get a boolean value
BOOL CIni::GetBool(LPCTSTR lpSection, LPCTSTR lpKey, BOOL bDefault) const
{
	TCHAR sz[DEF_PROFILE_NUM_LEN + 1] = _T("");
	GetString(lpSection, lpKey, sz, DEF_PROFILE_NUM_LEN);
	return StringToBool(sz, bDefault);
}

// Get a double floating value
double CIni::GetDouble(LPCTSTR lpSection, LPCTSTR lpKey, double fDefault) const
{
	TCHAR sz[DEF_PROFILE_NUM_LEN + 1] = _T("");
	GetString(lpSection, lpKey, sz, DEF_PROFILE_NUM_LEN);
	return *sz == _T('\0') ? fDefault : _tcstod(sz, NULL);
}

// Write a signed integral value to the ini file
BOOL CIni::WriteInt(LPCTSTR lpSection, LPCTSTR lpKey, int nValue, int nBase) const
{
	TCHAR szValue[DEF_PROFILE_NUM_LEN + 1] = _T("");
	__IntToString(nValue, szValue, nBase);
	return WriteString(lpSection, lpKey, szValue);
}

// Write an unsigned value to the ini file
BOOL CIni::WriteUInt(LPCTSTR lpSection, LPCTSTR lpKey, UINT nValue, int nBase) const
{
	TCHAR szValue[DEF_PROFILE_NUM_LEN + 1] = _T("");
	__UIntToString(nValue, szValue, nBase);
	return WriteString(lpSection, lpKey, szValue);
}

// Write a double floating value to the ini file
BOOL CIni::WriteDouble(LPCTSTR lpSection, LPCTSTR lpKey, double fValue, int nPrecision) const
{
	TCHAR szFmt[16] = _T("%f");

	if (nPrecision > 0)
		_stprintf(szFmt, _T("%%.%df"), nPrecision);

	TCHAR szValue[DEF_PROFILE_NUM_LEN + 1] = _T("");
	_stprintf(szValue, szFmt, fValue);
	return WriteString(lpSection, lpKey, szValue);
}

// Read a double value from the ini file, increase it then write it back
BOOL CIni::IncreaseDouble(LPCTSTR lpSection, LPCTSTR lpKey, double fIncrease, int nPrecision) const
{
	double f = GetDouble(lpSection, lpKey, 0.0);
	f += fIncrease;
	return WriteDouble(lpSection, lpKey, f, nPrecision);
}

// Write a boolean value to the ini file
BOOL CIni::WriteBool(LPCTSTR lpSection, LPCTSTR lpKey, BOOL bValue) const
{
	return WriteInt(lpSection, lpKey, bValue ? 1 : 0, BASE_DECIMAL);
}

// Read a boolean value from the ini file, invert it(true becomes false, false becomes true),
// then write it back
BOOL CIni::InvertBool(LPCTSTR lpSection, LPCTSTR lpKey) const
{
	return WriteBool(lpSection, lpKey, !GetBool(lpSection, lpKey, FALSE));
}

// Read a int from the ini file, increase it and then write it back to the ini file
BOOL CIni::IncreaseInt(LPCTSTR lpSection, LPCTSTR lpKey, int nIncrease, int nBase) const
{
	int nVal = GetInt(lpSection, lpKey, 0, nBase);
	nVal += nIncrease;
	return WriteInt(lpSection, lpKey, nVal, nBase);
}

// Read an UINT from the ini file, increase it and then write it back to the ini file
BOOL CIni::IncreaseUInt(LPCTSTR lpSection, LPCTSTR lpKey, UINT nIncrease, int nBase) const
{
	UINT nVal = GetUInt(lpSection, lpKey, 0, nBase);
	nVal += nIncrease;
	return WriteUInt(lpSection, lpKey, nVal, nBase);
}

TCHAR CIni::GetChar(LPCTSTR lpSection, LPCTSTR lpKey, TCHAR cDefault) const
{
	TCHAR sz[2] = _T("");
	GetString(lpSection, lpKey, sz, 1);
	return *sz == _T('\0') ? cDefault : sz[0];
}

BOOL CIni::WriteChar(LPCTSTR lpSection, LPCTSTR lpKey, TCHAR c) const
{
	TCHAR sz[2] = { c, _T('\0') };
	return WriteString(lpSection, lpKey, sz);
}

/////////////////////////////////////////////////////////////////////////////////
// User-Defined Data Type Access
/////////////////////////////////////////////////////////////////////////////////

// Get a block of raw data from the ini file
DWORD CIni::GetDataBlock(LPCTSTR lpSection, LPCTSTR lpKey, LPVOID lpBuffer, DWORD dwBufSize, DWORD dwOffset) const
{
	LPTSTR psz = __GetStringDynamic(lpSection, lpKey);
	DWORD dwLen = _tcslen(psz) / 2;
	if (dwLen <= dwOffset)
	{
		delete [] psz;
		return 0;
	}

	// verify psz, must be all in hex format
	for (int i = 0; psz[i] != _T('\0'); i++)
	{
		TCHAR c = psz[i];
		if ((c >= _T('0') && c <= _T('9'))
			|| (c >= _T('a') && c <= _T('f'))
			|| (c >= _T('A') && c <= _T('F')))
		{
			// valid
		}
		else
		{
			delete [] psz;
			return 0;
		}
	}

	DWORD dwProcLen = 0;
	LPBYTE lpb = (LPBYTE)lpBuffer;

	if (lpb != NULL)
	{
		dwProcLen = min(dwLen - dwOffset, dwBufSize);
		LPCTSTR p = &psz[dwOffset * 2];
		for (DWORD i = 0; i < dwProcLen; i++)
		{			
			TCHAR sz[3] = _T("");
			_tcsncpy(sz, p, 2);			
			lpb[i] = BYTE(_tcstoul(sz, NULL, 16));
			p = &p[2];
		}			
	}
	else
	{
		dwProcLen = dwLen - dwOffset;
	}
	delete [] psz;
	return dwProcLen;
}

// Write a block of raw data to the ini file
BOOL CIni::WriteDataBlock(LPCTSTR lpSection, LPCTSTR lpKey, LPCVOID lpData, DWORD dwDataSize) const
{
	const BYTE* lpb = (const BYTE*)lpData;
	if (lpb == NULL)
		return FALSE;

	LPTSTR psz = new TCHAR[dwDataSize * 2 + 1];
	for (DWORD i = 0, j = 0; i < dwDataSize; i++, j += 2)
		_stprintf(&psz[j], _T("%02X"), lpb[i]);
	const BOOL RES = WriteString(lpSection, lpKey, psz);
	delete [] psz;
	return RES;
}

// Append a block of raw data to a specified key in the ini file
BOOL CIni::AppendDataBlock(LPCTSTR lpSection, LPCTSTR lpKey, LPCVOID lpData, DWORD dwDataSize) const
{
	const BYTE* lpb = (const BYTE*)lpData;
	if (lpb == NULL)
		return FALSE;

	LPTSTR psz = new TCHAR[dwDataSize * 2 + 1];
	for (DWORD i = 0, j = 0; i < dwDataSize; i++, j += 2)
		_stprintf(&psz[j], _T("%02X"), lpb[i]);
	const BOOL RES = AppendString(lpSection, lpKey, psz);
	delete [] psz;
	return RES;
}

// Get a POINT value
POINT CIni::GetPoint(LPCTSTR lpSection, LPCTSTR lpKey, POINT ptDefault) const
{
	POINT pt;
	if (GetDataBlock(lpSection, lpKey, &pt, sizeof(POINT)) != sizeof(POINT))
		pt = ptDefault;
	return pt;
}

// Get a RECT value
RECT CIni::GetRect(LPCTSTR lpSection, LPCTSTR lpKey, RECT rcDefault) const
{
	RECT rc;
	if (GetDataBlock(lpSection, lpKey, &rc, sizeof(RECT)) != sizeof(RECT))
		rc = rcDefault;
	return rc;
}

// Write a POINT to the ini file
BOOL CIni::WritePoint(LPCTSTR lpSection, LPCTSTR lpKey, POINT pt) const
{
	return WriteDataBlock(lpSection, lpKey, &pt, sizeof(POINT));
}

// Write a RECT to the ini file
BOOL CIni::WriteRect(LPCTSTR lpSection, LPCTSTR lpKey, RECT rc) const
{
	return WriteDataBlock(lpSection, lpKey, &rc, sizeof(RECT));
}

/////////////////////////////////////////////////////////////////////////////////
// Sections & Keys Access
/////////////////////////////////////////////////////////////////////////////////

// Retrieve a list of key-lines(key-pairs) of the specified section
DWORD CIni::GetKeyLines(LPCTSTR lpSection, LPTSTR lpBuffer, DWORD dwBufSize) const
{
	if (lpBuffer != NULL)
		*lpBuffer = _T('\0');

	if (lpSection == NULL)
		return 0;	

	if (lpBuffer == NULL)
	{
		// just calculate the required buffer size
		DWORD dwLen = DEF_PROFILE_THRESHOLD;
		LPTSTR psz = new TCHAR[dwLen + 1];
		DWORD dwCopied = ::GetPrivateProfileSection(lpSection, psz, dwLen, m_pszPathName);

		while (dwCopied + 2 >= dwLen)
		{
			dwLen += DEF_PROFILE_THRESHOLD;
			delete [] psz;
			psz = new TCHAR[dwLen + 1];
			dwCopied = ::GetPrivateProfileSection(lpSection, psz, dwLen, m_pszPathName);
		}

		delete [] psz;
		return dwCopied + 2;
	}
	else
	{
		return ::GetPrivateProfileSection(lpSection, lpBuffer, dwBufSize, m_pszPathName);
	}
}

// Retrieve a list of key names of the specified section
DWORD CIni::GetKeyNames(LPCTSTR lpSection, LPTSTR lpBuffer, DWORD dwBufSize) const
{
	if (lpBuffer != NULL)
		*lpBuffer = _T('\0');

	if (lpSection == NULL)
		return 0;	

	STR_LIMIT sl;	
	sl.lpTarget = lpBuffer;
	sl.dwRemain = dwBufSize;
	sl.dwTotalCopied = 0;

	const DWORD LEN = GetKeyLines(lpSection, NULL, 0);
	if (LEN == 0)
		return 0;

	LPTSTR psz = new TCHAR[LEN + 1];
	GetKeyLines(lpSection, psz, LEN);
	ParseDNTString(psz, __KeyPairProc, (LPVOID)(&sl));
	delete [] psz;
	if (lpBuffer != NULL)
		lpBuffer[sl.dwTotalCopied] = _T('\0');
	return sl.dwTotalCopied;
}

// Get all section names from an ini file
DWORD CIni::GetSectionNames(LPTSTR lpBuffer, DWORD dwBufSize) const
{
	if (lpBuffer == NULL)
	{
		// just calculate the required buffer size
		DWORD dwLen = DEF_PROFILE_THRESHOLD;
		LPTSTR psz = new TCHAR[dwLen + 1];
		DWORD dwCopied = ::GetPrivateProfileSectionNames(psz, dwLen, m_pszPathName);
		while (dwCopied + 2 >= dwLen)
		{
			dwLen += DEF_PROFILE_THRESHOLD;
			delete [] psz;
			psz = new TCHAR[dwLen + 1];
			dwCopied = ::GetPrivateProfileSectionNames(psz, dwLen, m_pszPathName);
		}
		
		delete [] psz;
		return dwCopied + 2;
	}
	else
	{
		return ::GetPrivateProfileSectionNames(lpBuffer, dwBufSize, m_pszPathName);
	}
}

#ifdef __AFXWIN_H__
void CIni::GetSectionNames(CStringArray *pArray) const
{
	if (pArray != NULL)
		pArray->RemoveAll();

	const DWORD LEN = GetSectionNames(NULL, 0);
	if (LEN == 0)
		return;

	LPTSTR psz = new TCHAR[LEN + 1];
	GetSectionNames(psz, LEN);
	ParseDNTString(psz, __SubStrAdd, pArray);
	delete [] psz;
}
#endif

#ifdef __AFXWIN_H__
// Retrieve a list of key-lines(key-pairs) of the specified section
void CIni::GetKeyLines(LPCTSTR lpSection, CStringArray *pArray) const
{
	if (pArray != NULL)
		pArray->RemoveAll();

	const DWORD LEN = GetKeyLines(lpSection, NULL, 0);
	if (LEN == 0)
		return;

	LPTSTR psz = new TCHAR[LEN + 1];
	GetKeyLines(lpSection, psz, LEN);
	ParseDNTString(psz, __SubStrAdd, pArray);
	delete [] psz;
}
#endif

#ifdef __AFXWIN_H__
// Retrieve a list of key names of the specified section
void CIni::GetKeyNames(LPCTSTR lpSection, CStringArray *pArray) const
{
	if (pArray == NULL)
		return;

	pArray->RemoveAll();
	const LEN = GetKeyNames(lpSection, NULL, 0);
	LPTSTR psz = new TCHAR[LEN + 1];
	GetKeyNames(lpSection, psz, LEN);
	ParseDNTString(psz, __SubStrAdd, (LPVOID)pArray);
	delete [] psz;
}
#endif

// Remove whole section from the ini file
BOOL CIni::DeleteSection(LPCTSTR lpSection) const
{
	return ::WritePrivateProfileString(lpSection, NULL, _T(""), m_pszPathName);
}

// Remove a key from a section
BOOL CIni::DeleteKey(LPCTSTR lpSection, LPCTSTR lpKey) const
{
	return ::WritePrivateProfileString(lpSection, lpKey, NULL, m_pszPathName);
}

BOOL CIni::IsSectionExist(LPCTSTR lpSection) const
{
	if (lpSection == NULL)
		return FALSE;

	// first get the section name list, then check if lpSection exists
	// in the list.
	const DWORD LEN = GetSectionNames(NULL, 0);
	if (LEN == 0)
		return FALSE;

	LPTSTR psz = new TCHAR[LEN + 1];
	GetSectionNames(psz, LEN);
	BOOL RES = !ParseDNTString(psz, __SubStrCompare, (LPVOID)lpSection);
	delete [] psz;
	return RES;
}

BOOL CIni::IsKeyExist(LPCTSTR lpSection, LPCTSTR lpKey) const
{
	if (lpSection == NULL || lpKey == NULL)
		return FALSE;

	// Test it with the default unique string
	LPTSTR psz = __GetStringDynamic(lpSection, lpKey, DEF_PROFILE_TESTSTRING);
	const BOOL RES = (_tcscmp(psz, DEF_PROFILE_TESTSTRING) != 0);
	delete [] psz;
	return RES;
}

BOOL CIni::CopySection(LPCTSTR lpSrcSection, LPCTSTR lpDestSection, BOOL bFailIfExist) const
{
	if (lpSrcSection == NULL || lpDestSection == NULL)
		return FALSE;

	if (_tcsicmp(lpSrcSection, lpDestSection) == 0)
		return FALSE;

	if (!IsSectionExist(lpSrcSection))
		return FALSE;

	if (bFailIfExist && IsSectionExist(lpDestSection))
		return FALSE;

	DeleteSection(lpDestSection);

	const DWORD SRC_LEN = GetKeyLines(lpSrcSection, NULL, 0);
	LPTSTR psz = new TCHAR[SRC_LEN + 2];
	//memset(psz, 0, sizeof(TCHAR) * (SRC_LEN + 2));
	GetKeyLines(lpSrcSection, psz, SRC_LEN);	
	const BOOL RES = ::WritePrivateProfileSection(lpDestSection, psz, m_pszPathName);
	delete [] psz;

	return RES;
}

BOOL CIni::CopyKey(LPCTSTR lpSrcSection, LPCTSTR lpSrcKey, LPCTSTR lpDestSection, LPCTSTR lpDestKey, BOOL bFailIfExist) const
{
	if (lpSrcSection == NULL || lpSrcKey == NULL || lpDestKey == NULL)
		return FALSE;

	if (_tcsicmp(lpSrcSection, lpDestSection) == 0
		&& _tcsicmp(lpSrcKey, lpDestKey) == 0)
		return FALSE;

	if (!IsKeyExist(lpSrcSection, lpSrcKey))
		return FALSE;

	if (bFailIfExist && IsKeyExist(lpDestSection, lpDestKey))
		return FALSE;
	
	LPTSTR psz = __GetStringDynamic(lpSrcSection, lpSrcKey);
	const BOOL RES = WriteString(lpDestSection, lpDestKey, psz);
	delete [] psz;
	return RES;
}

BOOL CIni::MoveSection(LPCTSTR lpSrcSection, LPCTSTR lpDestSection, BOOL bFailIfExist) const
{
	return CopySection(lpSrcSection, lpDestSection, bFailIfExist)
		&& DeleteSection(lpSrcSection);
}

BOOL CIni::MoveKey(LPCTSTR lpSrcSection, LPCTSTR lpSrcKey, LPCTSTR lpDestSection, LPCTSTR lpDestKey, BOOL bFailIfExist) const
{
	return CopyKey(lpSrcSection, lpSrcKey, lpDestSection, lpDestKey, bFailIfExist)
		&& DeleteKey(lpSrcSection, lpSrcKey);
}

/////////////////////////////////////////////////////////////////////////////////
// Helper Functions
/////////////////////////////////////////////////////////////////////////////////

// Get a profile string value, return a heap pointer so we do not have to worry
// about the buffer size, however, this function requires the caller to manually
// free the memory.
// This function is the back-bone of all "Getxxx" functions of this class.
LPTSTR CIni::__GetStringDynamic(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpDefault) const
{
	TCHAR* psz = NULL;
	if (lpSection == NULL || lpKey == NULL)
	{
		// Invalid section or key name, just return the default string
		if (lpDefault == NULL)
		{
			// Empty string
			psz = new TCHAR[1];
			*psz = _T('\0');
		}
		else
		{
			psz = new TCHAR[_tcslen(lpDefault) + 1];
			_tcscpy(psz, lpDefault);
		}
		
		return psz;
	}
	
	// Keep enlarging the buffer size until being certain on that the string we
	// retrieved was original(not truncated).
	DWORD dwLen = DEF_PROFILE_THRESHOLD;
	psz = new TCHAR[dwLen + 1];
	DWORD dwCopied = ::GetPrivateProfileString(lpSection, lpKey, lpDefault == NULL ? _T("") : lpDefault, psz, dwLen, m_pszPathName);
	while (dwCopied + 1 >= dwLen)
	{		
		dwLen += DEF_PROFILE_THRESHOLD;
		delete [] psz;
		psz = new TCHAR[dwLen + 1];
		dwCopied = ::GetPrivateProfileString(lpSection, lpKey, lpDefault == NULL ? _T("") : lpDefault, psz, dwLen, m_pszPathName);
	}
	
	return psz; // !!! Requires the caller to free this memory !!!
}

// Split a string usinf a particular delimiter, split result are copied into lpBuffer
// in the "double null terminated string" format as the following figure shows:
// xxx\0xxxx\0xx\0xxx\0\0
//
// For example, if the delimiter is ",", then string "ab,cd,e" will be
// splitted into "ab\0cd\0e\0\0", this string format can be parsed into an array
// of sub strings easily using user defined functions or CIni::ParseStringArray.
DWORD CIni::__StringSplit(LPCTSTR lpString, LPTSTR lpBuffer, DWORD dwBufSize, LPCTSTR lpDelimiter, BOOL bTrimString)
{
	if (lpString == NULL || lpBuffer == NULL || dwBufSize == 0)
		return 0;	

	DWORD dwCopied = 0;
	*lpBuffer = _T('\0');
	if (*lpString == _T('\0'))
		return 0;

	// If lpDelimiter is NULL, use the default delimiter ",", if delimiter length
	// is 0, then return whole string
	if (lpDelimiter != NULL && *lpDelimiter == _T('\0'))
	{
		_tcsncpy(lpBuffer, lpString, dwBufSize - 1);
		return _tcslen(lpBuffer);
	}

	LPTSTR pszDel = (lpDelimiter == NULL) ? _tcsdup(DEF_PROFILE_DELIMITER) : _tcsdup(lpDelimiter);
	const DWORD DEL_LEN = _tcslen(pszDel);
	LPTSTR lpTarget = lpBuffer;

	// Search through lpString for delimiter matches, and extract sub strings out
	LPCTSTR lpPos = lpString;
	LPCTSTR lpEnd = _tcsstr(lpPos, pszDel);

	while (lpEnd != NULL)
	{
		LPTSTR pszSeg = __StrDupEx(lpPos, lpEnd);
		if (bTrimString)
			__TrimString(pszSeg);

		const DWORD SEG_LEN = _tcslen(pszSeg);
		const DWORD COPY_LEN = min(SEG_LEN, dwBufSize - dwCopied);

		// Need to avoid buffer overflow
		if (COPY_LEN > 0)
		{
			dwCopied += COPY_LEN + 1;
			_tcsncpy(lpTarget, pszSeg, COPY_LEN);
			lpTarget[COPY_LEN] = _T('\0');
			lpTarget = &lpTarget[SEG_LEN + 1];
		}
		delete [] pszSeg;
		lpPos = &lpEnd[DEL_LEN]; // Advance the pointer for next search		
		lpEnd = _tcsstr(lpPos, pszDel);
	}

	// The last part of string, there may not be the trailing delimiter, so we
	// need to take care of this part, too
	LPTSTR pszSeg = _tcsdup(lpPos);
	if (bTrimString)
		__TrimString(pszSeg);

	const DWORD SEG_LEN = _tcslen(pszSeg);
	const DWORD COPY_LEN = min(SEG_LEN, dwBufSize - dwCopied);

	if (COPY_LEN > 0)
	{
		dwCopied += COPY_LEN + 1;
		_tcsncpy(lpTarget, pszSeg, COPY_LEN);
		lpTarget[COPY_LEN] = _T('\0');
	}

	delete [] pszSeg;
	lpBuffer[dwCopied] = _T('\0');
	delete [] pszDel;
	return dwCopied;
}

// Parse a "double null terminated string", pass each sub string to a user-defined
// callback function
BOOL CIni::ParseDNTString(LPCTSTR lpString, SUBSTRPROC lpFnStrProc, LPVOID lpParam)
{
	if (lpString == NULL || lpFnStrProc == NULL)
		return FALSE;

	LPCTSTR p = lpString;
	DWORD dwLen = _tcslen(p);

	while (dwLen > 0)
	{
		if (!lpFnStrProc(p, lpParam))
			return FALSE;

		p = &p[dwLen + 1];
		dwLen = _tcslen(p);
	}
	return TRUE;
}

// Callback function used to compare elements inside of a 
// "double null terminated string" with a given string. Useful for
// searching in the section names list.
BOOL CALLBACK CIni::__SubStrCompare(LPCTSTR lpString1, LPVOID lpParam)
{
	assert(lpString1 != NULL);
	LPCTSTR lpString2 = (LPCTSTR)lpParam;
	assert(lpString2 != NULL);
	// if two string matches, return zero to stop the parsing
	return _tcsicmp(lpString1, lpString2) != 0;
}

// Callback function used to process a key-pair, it extracts the
// key name from the key-pair string
BOOL CALLBACK CIni:: __KeyPairProc(LPCTSTR lpString, LPVOID lpParam)
{
	STR_LIMIT* psl = (STR_LIMIT*)lpParam;
	if (lpString == NULL || psl== NULL)
		return FALSE;
	
	LPCTSTR p = _tcschr(lpString, _T('='));
	if (p == NULL || p == lpString)
		return TRUE;
	
	// extract the sub-string on left side of the '='
	LPTSTR psz = new TCHAR[_tcslen(lpString) + 1];
	for (int i = 0; &lpString[i] < p; i++)
		psz[i] = lpString[i];
	psz[i] = _T('\0');

	// trim
	__TrimString(psz);
	DWORD dwNameLen = _tcslen(psz);
	DWORD dwCopyLen = 0;
	
	//copy to the buffer
	if (psl->lpTarget != NULL)
	{
		dwCopyLen = (psl->dwRemain > 1) ? min(dwNameLen, psl->dwRemain - 1) : 0;
		_tcsncpy(psl->lpTarget, psz, dwCopyLen);
		psl->lpTarget[dwCopyLen] = _T('\0');
		psl->lpTarget = &(psl->lpTarget[dwCopyLen + 1]); 
		psl->dwRemain -= dwCopyLen + 1;
	}
	else
	{
		dwCopyLen = dwNameLen;
	}

	delete [] psz;
	psl->dwTotalCopied += dwCopyLen + 1;
	return TRUE;
}

#ifdef __AFXWIN_H__
// Callback function used to add elements that are extracted from a 
// "double null terminated string" to an MFC CStringArray.
BOOL CALLBACK CIni::__SubStrAdd(LPCTSTR lpString, LPVOID lpParam)
{
	CStringArray* pArray = (CStringArray*)lpParam;
	if (pArray == NULL || lpString == NULL)
		return FALSE;

	pArray->Add(lpString);
	return TRUE;
}
#endif

// Convert an integer into binary string format
void CIni::__ToBinaryString(UINT nNumber, LPTSTR lpBuffer, DWORD dwBufSize)
{
	if (dwBufSize == 0)
		return;
	
	DWORD dwIndex = 0;	
	do
	{
		lpBuffer[dwIndex++] = (nNumber % 2) ? _T('1') : _T('0');
		nNumber /= 2;
	} while (nNumber > 0 && dwIndex < dwBufSize);

	lpBuffer[dwIndex] = _T('\0');
	_tcsrev(lpBuffer);
}

// Make sure the base will be expected value
int CIni::__ValidateBase(int nBase)
{
	switch (nBase)
	{
	case BASE_BINARY:
	case BASE_OCTAL:
	case BASE_HEXADECIMAL:
		break;

	default:
		nBase = BASE_DECIMAL;
	}

	return nBase;
}

// Convert a signed integer into string representation, based on its base
void CIni::__IntToString(int nNumber, LPTSTR lpBuffer, int nBase)
{
	switch (nBase)
	{
	case BASE_BINARY:
	case BASE_OCTAL:
	case BASE_HEXADECIMAL:
		__UIntToString((UINT)nNumber, lpBuffer, nBase);
		break;

	default:
		_stprintf(lpBuffer, _T("%d"), nNumber);
		break;
	}	
}

// Convert an unsigned integer into string representation, based on its base
void CIni::__UIntToString(UINT nNumber, LPTSTR lpBuffer, int nBase)
{
	switch (nBase)
	{
	case BASE_BINARY:
		__ToBinaryString(nNumber, lpBuffer, DEF_PROFILE_NUM_LEN);
		break;

	case BASE_OCTAL:
		_stprintf(lpBuffer, _T("%o"), nNumber);
		break;

	case BASE_HEXADECIMAL:
		_stprintf(lpBuffer, _T("%X"), nNumber);
		break;

	default:
		_stprintf(lpBuffer, _T("%u"), nNumber);
		break;
	}	
}

BOOL CIni::StringToBool(LPCTSTR lpString, BOOL bDefault)
{
	// Default: empty string
	// TRUE: "true", "yes", non-zero decimal numner
	// FALSE: all other cases
	if (lpString == NULL || *lpString == _T('\0'))
		return bDefault;

	return (_tcsicmp(lpString, _T("true")) == 0
		|| _tcsicmp(lpString, _T("yes")) == 0
		|| _tcstol(lpString, NULL, BASE_DECIMAL) != 0);
}

BOOL CIni::__TrimString(LPTSTR lpString)
{
	if (lpString == NULL)
		return FALSE;

	BOOL bTrimmed = FALSE;
	int nLen = _tcslen(lpString);

	// '\n' and '\r' are actually not possible in this case, but anyway...
	
	// Trim right side
	while (nLen >= 0
		&& (lpString[nLen - 1] == _T(' ')
			|| lpString[nLen - 1] == _T('\t')
			|| lpString[nLen - 1] == _T('\r')
			|| lpString[nLen - 1] == _T('\n')))
	{
		lpString[--nLen] = _T('\0');
		bTrimmed = TRUE;		
	}

	// Trim left side
	LPCTSTR p = lpString; 
	while (*p == _T(' ')
			|| *p == _T('\t')
			|| *p == _T('\r')
			|| *p == _T('\n'))
	{
		p = &p[1];
		bTrimmed = TRUE;
	}

	if (p != lpString)
	{
		LPTSTR psz = _tcsdup(p);
		_tcscpy(lpString, psz);
		delete [] psz;
	}

	return bTrimmed;
}

LPTSTR CIni::__StrDupEx(LPCTSTR lpStart, LPCTSTR lpEnd)
{
	const DWORD LEN = ((DWORD)lpEnd - (DWORD)lpStart) / sizeof(TCHAR);
	LPTSTR psz = new TCHAR[LEN + 1];
	_tcsncpy(psz, lpStart, LEN);
	psz[LEN] = _T('\0');
	return psz; // !!! Requires the caller to free this memory !!!
}

/////////////////////////////////////////////////////////////////////////////////
// End of Cini Class Implementation
/////////////////////////////////////////////////////////////////////////////////

// If you are getting this error:
// ----------------------------------------------------------------------------
// "fatal error C1010: unexpected end of file while looking for precompiled
//  header directive"
//-----------------------------------------------------------------------------
// Please scroll all the way up and uncomment '#include "stdafx.h"'
