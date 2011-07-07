// Schedule3MonthInfo.h: interface for the CSchedule3MonthInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCHEDULE3MONTHINFO_H__887855AE_162D_4CD0_BF3E_B9CE4C55C3D1__INCLUDED_)
#define AFX_SCHEDULE3MONTHINFO_H__887855AE_162D_4CD0_BF3E_B9CE4C55C3D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "OrganizeDeclaration.h"
#include "ScheduleAllInfo.h"
#include "ScheduleMonthInfo.h"

class CSchedule3MonthInfo  
{
public:
	CSchedule3MonthInfo();
	virtual ~CSchedule3MonthInfo();

public:
	CScheduleMonthInfo LastMonth;
	CScheduleMonthInfo CurrentMonth;
	CScheduleMonthInfo NextMonth;
	CScheduleAllInfo AllSchedule;
	CScheduleAllRepeatInfo repeatSchedule;

public:
	void RemoveScheduleInfo(CScheduleInfo& si);
	void GetRepeatOriginalInfo(CScheduleInfo &si);
	void RepeatDataExtend();
	void UpdateScheduleData();
	CScheduleMonthInfo* GetScheduleMonthInfo(const COleDateTimeEx& dt);
	void SetScheduleData(SchedulesRecord *psr,int rc);
	void GetScheduleData(SchedulesRecord *psr,int &rc);
	void GetRecordCount(int &rc);
private:
	BOOL xIsSameMonth(CScheduleMonthInfo& mi,const COleDateTimeEx& dt);
	void xResetAllScheduleData();
	void xSaveAllScheduleData();
	void xSaveScheduleData(CScheduleMonthInfo& mi);
	void xLoadAllScheduleData(COleDateTimeEx dt);
	void xLoadScheduleData(CScheduleMonthInfo& mi);
};

#endif // !defined(AFX_SCHEDULE3MONTHINFO_H__887855AE_162D_4CD0_BF3E_B9CE4C55C3D1__INCLUDED_)
