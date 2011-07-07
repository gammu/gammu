// ScheduleMonthInfo.h: interface for the CScheduleMonthInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCHEDULEMONTHINFO_H__A510DA8A_F910_46C8_81FC_50BEE9F82CCB__INCLUDED_)
#define AFX_SCHEDULEMONTHINFO_H__A510DA8A_F910_46C8_81FC_50BEE9F82CCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "OrganizeDeclaration.h"
#include "ScheduleDayInfo.h"
#include "ScheduleInfo.h"
#include "ScheduleTimeInfo.h"

class CScheduleMonthInfo  
{
public:
	CScheduleMonthInfo();
	virtual ~CScheduleMonthInfo();

public:
	CScheduleMonthInfo(CScheduleMonthInfo &si);
	COleDateTimeEx GetCurrentDate();
	void Reset();
	void SaveScheduleMonthData(CList<SchedulesRecord,SchedulesRecord&> &srList);
	void GetDataCount(int &cnt);
	void GetRowCount(int &cnt);
	void GetDateString(CString &cs);
	void ParseDateTimeString(CString &cs,int &year,int &month,int &day,
		                       int &hour,int &min,int &sec);
	void ParseDateString(CString &cs,int &year,int &month,int &day) ;
	BOOL IsSameYearMonth(int year,int month);
	BOOL IsSameDate(int year,int month,int day);
	BOOL ParseTimeString(CString &inStr,int &hour,int &min,int &sec);
	void SetScheduleDate(COleDateTimeEx dt);
	CScheduleDayInfo GetCurrentDayInfo();
	CScheduleMultiInfo GetInfo(int row);
	void GetScheduleInfo(int row,CScheduleInfo &si);
	//	CScheduleInfo GetInfo(int row);
	BOOL GetFirstWorkingHourInfo(CScheduleInfo &si);
		//	BOOL GetFirstEmptyInfo(CScheduleInfo &si);
	BOOL GetUpdatedInfo(int selectedIndex,CScheduleInfo &si);
	//	CScheduleInfo GetInfoAndResetInfo(int row);
	void ResetScheduleInfo(int row);
	BOOL RemoveScheduleInfo(CScheduleInfo &si);
	BOOL AddScheduleInfo(CScheduleInfo &si , bool bExtend = false);
	BOOL AddScheduleInfoBeforeSame(CScheduleInfo &si);
	//	BOOL SetInfo(CScheduleInfo &si);
	void SetDirtyInfo(CScheduleInfo si,BOOL bDirty);
	BOOL CheckDirty(CScheduleInfo si);
	CScheduleTimeInfo GetStopTime(CScheduleInfo sti);
	void SetDate(int year,int month);
	void SetNextMonth();
	void SetPreMonth();
	//	BOOL CheckConflict(CScheduleInfo sti,int orgRow);
	int GetInfoSize();
	void GetCurrentDayRowCountBeforeTime(int nTimeRow,int &cnt) ;
public:
	int CurrentYear;
	int CurrentMonth;
	int CurrentDay;
	CScheduleDayInfo sDayInfo[31];
};

#endif // !defined(AFX_SCHEDULEMONTHINFO_H__A510DA8A_F910_46C8_81FC_50BEE9F82CCB__INCLUDED_)
