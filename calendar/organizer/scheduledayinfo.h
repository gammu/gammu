// ScheduleDayInfo.h: interface for the CScheduleDayInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCHEDULEDAYINFO_H__74257D9B_24D4_4F51_9E31_2A4C7CC73721__INCLUDED_)
#define AFX_SCHEDULEDAYINFO_H__74257D9B_24D4_4F51_9E31_2A4C7CC73721__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "OrganizeDeclaration.h"
#include "ScheduleMultiInfo.h"
#include "ScheduleInfo.h"
#include "ScheduleTimeInfo.h"

class CScheduleDayInfo  
{
public:
	CScheduleDayInfo();
	virtual ~CScheduleDayInfo();

public:
	CScheduleDayInfo(CScheduleDayInfo &sdi);
	void SaveScheduleMonthData(CString dateString,CList<SchedulesRecord,SchedulesRecord&> &srList);
	void GetDataCount(int &cnt);
	void GetRowCount(int &cnt);
	CScheduleTimeInfo xGetScheduleTime(int tiIndex);
	CScheduleTimeInfo GetStopTime(CScheduleInfo sti);
	void Reset();
	int GetInfoSize();

public:
	void SetData(CScheduleDayInfo &sdi);
	BOOL bDirtyFlag[48];
	CScheduleMultiInfo sInfo[48];
};

#endif // !defined(AFX_SCHEDULEDAYINFO_H__74257D9B_24D4_4F51_9E31_2A4C7CC73721__INCLUDED_)
