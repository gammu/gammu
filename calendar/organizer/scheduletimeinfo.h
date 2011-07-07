// ScheduleTimeInfo.h: interface for the CScheduleTimeInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCHEDULETIMEINFO_H__C5E904C2_5FAD_4E1F_8A90_F2BBE0BFDCD7__INCLUDED_)
#define AFX_SCHEDULETIMEINFO_H__C5E904C2_5FAD_4E1F_8A90_F2BBE0BFDCD7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "OrganizeDeclaration.h"

class CScheduleTimeInfo  
{
public:
	CScheduleTimeInfo();
	CScheduleTimeInfo(CScheduleTimeInfo &sti);
	virtual ~CScheduleTimeInfo();

public:
	const BOOL operator !=(CScheduleTimeInfo &sti);
	const BOOL operator <=(CScheduleTimeInfo &sti);
	const BOOL operator <(CScheduleTimeInfo &sti);
	void MoveToNextHalfHour();
	void Reset();
	void MoveToNearestStartTime();
	void MoveToNearestStopTime();
	void SetData(CScheduleTimeInfo &sti);
	const CScheduleTimeInfo& operator =(const CScheduleTimeInfo &sti);
	BOOL ParseAMPMTimeString(CString inStr);
	CString GetAMPMTimeString();
	void SetTimeInfoByHourCount(int hhc);
	int GetHalfHourCount() ;
	void SetData(int h,int m,int s);
	CString FormatString();
	CString toString();
	CString GetTimeString();
	BOOL ParseTimeString(CString inStr,int &hour,int &min,int &sec);
//	BOOL ParseString(CString inStr);
    void Get24HourTime(int& nHour, int& nMinute);

public:
	int hour;
	int min;
	HALF_DAY halfDay;

};

#endif // !defined(AFX_SCHEDULETIMEINFO_H__C5E904C2_5FAD_4E1F_8A90_F2BBE0BFDCD7__INCLUDED_)
