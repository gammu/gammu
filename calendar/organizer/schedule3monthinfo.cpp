// Schedule3MonthInfo.cpp: implementation of the CSchedule3MonthInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Schedule3MonthInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSchedule3MonthInfo::CSchedule3MonthInfo()
{
	COleDateTimeEx dt = COleDateTime::GetCurrentTime();
	xLoadAllScheduleData(dt);
}

CSchedule3MonthInfo::~CSchedule3MonthInfo()
{

}

CScheduleMonthInfo* CSchedule3MonthInfo::GetScheduleMonthInfo(const COleDateTimeEx& dt) {
	if (xIsSameMonth(CurrentMonth,dt)) {
		CurrentMonth.CurrentDay = dt.GetDay();
		return &CurrentMonth;
	}
		if (xIsSameMonth(LastMonth,dt)) {
			LastMonth.CurrentDay = dt.GetDay();
			return &LastMonth;
		}
		if (xIsSameMonth(NextMonth,dt)) {
			NextMonth.CurrentDay = dt.GetDay();
			return &NextMonth;
		}
	xSaveAllScheduleData();
#ifdef _DEBUG
DumpInsertSchedule(&(AllSchedule.m_ScheduleRecord));
#endif

	xResetAllScheduleData();

#ifdef _DEBUG
DumpInsertSchedule(&(AllSchedule.m_ScheduleRecord));
#endif

	xLoadAllScheduleData(dt);

#ifdef _DEBUG
DumpInsertSchedule(&(AllSchedule.m_ScheduleRecord));
#endif
	CurrentMonth.SetScheduleDate(dt);

#ifdef _DEBUG
DumpInsertSchedule(&(AllSchedule.m_ScheduleRecord));
#endif

	return &CurrentMonth;
}
void CSchedule3MonthInfo::SetScheduleData(SchedulesRecord *psr,int rc) 
{   //下面函数引起crash kerm 1.07
	AllSchedule.SetScheduleData(psr,rc);
	COleDateTimeEx dt;
	dt.SetDate(CurrentMonth.CurrentYear,CurrentMonth.CurrentMonth,CurrentMonth.CurrentDay);
	//kerm 1.07?
	xLoadAllScheduleData(dt);

}

void CSchedule3MonthInfo::UpdateScheduleData() {
	xSaveAllScheduleData();
	COleDateTimeEx dt;
	dt.SetDate(CurrentMonth.CurrentYear,CurrentMonth.CurrentMonth,CurrentMonth.CurrentDay);
	xLoadAllScheduleData(dt);
}

void CSchedule3MonthInfo::RepeatDataExtend() {
	repeatSchedule.RemoveAllRepeatScheduleInfo();
	AllSchedule.LoadAllRepeatScheduleData(repeatSchedule);
}

void CSchedule3MonthInfo::GetScheduleData(SchedulesRecord *psr,int &rc) {
	xSaveAllScheduleData();
	AllSchedule.GetScheduleData(psr,rc);
}

void CSchedule3MonthInfo::GetRecordCount(int &rc) {
	xSaveAllScheduleData();
	AllSchedule.GetRecordCount(rc);
}

//private function
BOOL CSchedule3MonthInfo::xIsSameMonth(CScheduleMonthInfo& mi,const COleDateTimeEx& dt) {
	if (mi.CurrentYear != dt.GetYear()) {
		return FALSE;
	}
	if (mi.CurrentMonth != dt.GetMonth()) {
		return FALSE;
	}
	return TRUE;
}
void CSchedule3MonthInfo::xResetAllScheduleData() {
		LastMonth.Reset();
	CurrentMonth.Reset();
		NextMonth.Reset();
}
void CSchedule3MonthInfo::xSaveAllScheduleData() {
		xSaveScheduleData(LastMonth);
	xSaveScheduleData(CurrentMonth);
		xSaveScheduleData(NextMonth);
}
void CSchedule3MonthInfo::xSaveScheduleData(CScheduleMonthInfo& mi) {
	AllSchedule.SaveScheduleMonthData(mi);
}
void CSchedule3MonthInfo::xLoadAllScheduleData(COleDateTimeEx dt) {
	RepeatDataExtend();

		COleDateTimeEx dtt1,dtt2;

	COleDateTimeEx dt2 = dt;

		COleDateTimeEx dt1 = dt;
		COleDateTimeSpan dts;
		dts.SetDateTimeSpan(5+dt1.GetDay(),0,0,0);
		dt1 -= dts;
		dtt1.SetDate(dt1.GetYear(),dt1.GetMonth(),1);
		dtt2.SetDate(dt2.GetYear(),dt2.GetMonth(),1);
		dts = dtt2 - dtt1;
		dt1.SetDate(dt1.GetYear(),dt1.GetMonth(),min(dts.GetDays(),dt2.GetDay()));
	
		COleDateTimeEx dt3 = dt;
		COleDateTimeEx dt4;
		dts.SetDateTimeSpan(35-dt3.GetDay(),0,0,0);  //next month
		dt3 += dts;
		dtt1.SetDate(dt3.GetYear(),dt3.GetMonth(),1);
		dts.SetDateTimeSpan(35-dt3.GetDay(),0,0,0);  //next 2 month
		dt4 = dt3 + dts;
		dtt2.SetDate(dt4.GetYear(),dt4.GetMonth(),1);
		dts = dtt2 - dtt1;
		dt3.SetDate(dt3.GetYear(),dt3.GetMonth(),min(dts.GetDays(),dt2.GetDay()));

		LastMonth.SetScheduleDate(dt1);
	CurrentMonth.SetScheduleDate(dt2);
		NextMonth.SetScheduleDate(dt3);

		xLoadScheduleData(LastMonth);
	xLoadScheduleData(CurrentMonth);
		xLoadScheduleData(NextMonth);
}
void CSchedule3MonthInfo::xLoadScheduleData(CScheduleMonthInfo& mi) {
	mi.Reset();
	AllSchedule.LoadScheduleMonthData(mi);
	repeatSchedule.LoadScheduleMonthData(mi);
}

void CSchedule3MonthInfo::GetRepeatOriginalInfo(CScheduleInfo &si) {
	repeatSchedule.GetRepeatOriginalInfo(si);
}

void CSchedule3MonthInfo::RemoveScheduleInfo(CScheduleInfo &si)
{
	AllSchedule.RemoveScheduleInfo(si);
	repeatSchedule.RemoveScheduleInfo(si);
}
