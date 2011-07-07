// ScheduleMultiInfo.h: interface for the CScheduleMultiInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCHEDULEMULTIINFO_H__C3462D86_DF68_41C3_BAFF_20D894083064__INCLUDED_)
#define AFX_SCHEDULEMULTIINFO_H__C3462D86_DF68_41C3_BAFF_20D894083064__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ScheduleInfo.h"

class CScheduleMultiInfo  
{
public:
	CScheduleMultiInfo();
	CScheduleMultiInfo(CScheduleMultiInfo &info);
	virtual ~CScheduleMultiInfo();

public:
	const CScheduleMultiInfo& operator =(CScheduleMultiInfo &smui);
	BOOL IsEmpty();
	int GetCount();
	int GetRowCount();
	void SetData(CScheduleMultiInfo &info);
	void Reset();
	int rowIndex;
	CScheduleTimeInfo hiStartTime;

	int CompareData(CScheduleInfo &info1,CScheduleInfo &info2);
	BOOL IsScheduleInfoExisted(CScheduleInfo &info);
	CScheduleInfo GetScheduleInfo(int idx);
	BOOL ReplaceScheduleInfo(int idx,CScheduleInfo &info);
	BOOL RemoveScheduleInfo(CScheduleInfo &info);
	BOOL RemoveScheduleInfo(int idx);
	void AddScheduleInfo(CScheduleInfo &info,bool bExtend = false);
	void AddScheduleInfoBeforeSame(CScheduleInfo &si);
	CList<CScheduleInfo,CScheduleInfo> m_ScheduleInfoList;
};

#endif // !defined(AFX_SCHEDULEMULTIINFO_H__C3462D86_DF68_41C3_BAFF_20D894083064__INCLUDED_)
