// ScheduleAllInfo.h: interface for the CScheduleAllInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCHEDULEALLINFO_H__C7472A02_D085_44B4_89E0_30374EB1B472__INCLUDED_)
#define AFX_SCHEDULEALLINFO_H__C7472A02_D085_44B4_89E0_30374EB1B472__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "OrganizeDeclaration.h"
#include "ScheduleMonthInfo.h"
#include "ScheduleAllRepeatInfo.h"

class CScheduleAllInfo  
{
public:
	CScheduleAllInfo();
	virtual ~CScheduleAllInfo();
public:
	void SetScheduleData(SchedulesRecord *psr,int rc);
	void LoadScheduleMonthData(CScheduleMonthInfo& mi);
	void GetScheduleData(SchedulesRecord *psr,int &rc);
	void GetMonthDataCount(CScheduleMonthInfo& mi,int &cnt);
	void MoveMonthBeforeScheduleData(CScheduleMonthInfo& mi,CList<SchedulesRecord,SchedulesRecord&> &pSrNew,CList<SchedulesRecord,SchedulesRecord&> &pSrOld);
	void MoveMonthAfterScheduleData(CScheduleMonthInfo& mi,CList<SchedulesRecord,SchedulesRecord&> &pSrNew,CList<SchedulesRecord,SchedulesRecord&> &pSrOld);
	void SaveScheduleMonthData(CScheduleMonthInfo& mi);
	void GetRecordCount(int &recordCount) {recordCount = m_ScheduleRecord.GetCount();}
	void LoadAllRepeatScheduleData(CScheduleAllRepeatInfo &ari);

public:
	void RemoveScheduleInfo(CScheduleInfo &si);
	CList<SchedulesRecord,SchedulesRecord&> m_ScheduleRecord;

};

#endif // !defined(AFX_SCHEDULEALLINFO_H__C7472A02_D085_44B4_89E0_30374EB1B472__INCLUDED_)
