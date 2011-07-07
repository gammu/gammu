// ScheduleAllRepeatInfo.h: interface for the CScheduleAllRepeatInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCHEDULEALLREPEATINFO_H__F4761238_F98F_4363_8F59_A5AEA2B44E45__INCLUDED_)
#define AFX_SCHEDULEALLREPEATINFO_H__F4761238_F98F_4363_8F59_A5AEA2B44E45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ScheduleInfo.h"
#include "ScheduleMonthInfo.h"

class CScheduleAllRepeatInfo  
{
public:
	void RemoveScheduleInfo(CScheduleInfo &si);
	void GetRepeatOriginalInfo(CScheduleInfo &si);
	void LoadScheduleMonthData(CScheduleMonthInfo& mi);
	void ExtendRepeatInfo(CScheduleInfo &si,int year,int month,CList<CScheduleInfo,CScheduleInfo> &extendList);
	void RemoveAllRepeatScheduleInfo();
	void RemoveRepeatScheduleInfo(CScheduleInfo &si);
	void AddRepeatScheduleInfo(CScheduleInfo & si);
	CScheduleAllRepeatInfo();
	virtual ~CScheduleAllRepeatInfo();
	BOOL IsDateExcessEndDate(CScheduleInfo &si,COleDateTimeEx dt);

private:
	CList<CScheduleInfo,CScheduleInfo> m_SiList;
};

#endif // !defined(AFX_SCHEDULEALLREPEATINFO_H__F4761238_F98F_4363_8F59_A5AEA2B44E45__INCLUDED_)
