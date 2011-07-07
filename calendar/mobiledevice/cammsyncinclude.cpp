#include "stdafx.h"
#include "CAMMSyncInclude.h"

CString GuidToString(GUID guid)
{
	CString string;
	string.Format(_T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"),
				guid.Data1,
				guid.Data2,
				guid.Data3,
				guid.Data4[0],
				guid.Data4[1],
				guid.Data4[2],
				guid.Data4[3],
				guid.Data4[4],
				guid.Data4[5],
				guid.Data4[6],
				guid.Data4[7]);
	return string;
}

void StringToGuid(CString szStr, GUID& inputGUID)
{
	GUID guid;
	TCHAR buf[MAX_PATH];
	lstrcpy(buf,szStr.GetBuffer(MAX_PATH));
	szStr.ReleaseBuffer();
	_stscanf(buf,_T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"),
				&guid.Data1,
				&guid.Data2,
				&guid.Data3,
				&guid.Data4[0],
				&guid.Data4[1],
				&guid.Data4[2],
				&guid.Data4[3],
				&guid.Data4[4],
				&guid.Data4[5],
				&guid.Data4[6],
				&guid.Data4[7]);

	memcpy(&inputGUID, &guid, sizeof(GUID));
	return;

}