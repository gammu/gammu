// OleDateTimeEx.cpp: implementation of the COleDateTimeEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OleDateTimeEx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COleDateTimeEx::COleDateTimeEx()
{

}


COleDateTimeEx::~COleDateTimeEx()
{

}
int COleDateTimeEx::MoveToPrevDayDate(int dayCnt)
{
	COleDateTimeSpan dts;
	dts.SetDateTimeSpan(dayCnt,0,0,0);
	*this -= dts;

	return 0;
}

int COleDateTimeEx::MoveToNextDayDate(int dayCnt)
{
	COleDateTimeSpan dts;
	dts.SetDateTimeSpan(dayCnt,0,0,0);
	*this += dts;

	return 0;
}


int COleDateTimeEx::MoveToNextWeekDate(int weekCnt)
{
	COleDateTimeSpan dts;
	dts.SetDateTimeSpan(7*weekCnt,0,0,0);
	*this += dts;

	return 0;
}


int COleDateTimeEx::MoveToNextMonthDate(int monthCnt,int day)
{
	for (int i=0;i<monthCnt;i++) {
		MoveToNextMonthDate(day);
	}

	return 0;
}

int COleDateTimeEx::MoveToNextMonthDate(int day)
{
	COleDateTimeEx dt2,dt3;
	COleDateTimeSpan dts;
	dt2.SetDate(GetYear(),GetMonth(),28);
	dts.SetDateTimeSpan(5,0,0,0);
	dt2 += dts;
	dt2.SetDate(dt2.GetYear(),dt2.GetMonth(),1);

	dt3.SetDate(dt2.GetYear(),dt2.GetMonth(),28);
	dt3 += dts;
	dt3.SetDate(dt3.GetYear(),dt3.GetMonth(),1);
	dts = dt3 - dt2;
	if (dts.GetDays() < day) {
		SetDate(dt2.GetYear(),dt2.GetMonth(),dts.GetDays());
	} else {
		SetDate(dt2.GetYear(),dt2.GetMonth(),day);
	}

	return 0;
}

int COleDateTimeEx::MoveToNextYearDate(int yearCnt,int day)
{
	for (int i=0;i<yearCnt;i++) {
		MoveToNextYearDate(day);
	}

	return 0;
}

int COleDateTimeEx::MoveToNextYearDate(int day)
{
	COleDateTimeEx dt2,dt3;
	COleDateTimeSpan dts;
	dt2.SetDate(GetYear(),12,28);
	dts.SetDateTimeSpan(5,0,0,0);
	dt2 += dts;
	dt2.SetDate(dt2.GetYear(),GetMonth(),1);

	dt3.SetDate(dt2.GetYear(),dt2.GetMonth(),28);
	dt3 += dts;
	dt3.SetDate(dt3.GetYear(),dt3.GetMonth(),1);
	dts = dt3 - dt2;
	if (dts.GetDays() < day) {
		SetDate(dt2.GetYear(),dt2.GetMonth(),dts.GetDays());
	} else {
		SetDate(dt2.GetYear(),dt2.GetMonth(),day);
	}

	return 0;
}

int COleDateTimeEx::MoveToNextMonthlyWeeklyDate(int monthCnt,int whichWeek,int whichDay,int day)
{
	for (int i=0;i<monthCnt;i++) {
		MoveToNextMonthlyWeeklyDate(whichWeek,whichDay,day);
	}

	return 0;
}

int COleDateTimeEx::MoveToFirstMonthlyWeeklyDate(int whichWeek,int whichDay,int day) {
	COleDateTimeEx dt2;
	COleDateTimeSpan dts;
	dt2.SetDate(GetYear(),GetMonth(),1);
	int doW2 = dt2.GetDayOfWeek();
	doW2--;

	if (whichDay < doW2) {
		dts.SetDateTimeSpan(whichDay-doW2+7,0,0,0);
	} else {
		dts.SetDateTimeSpan(whichDay-doW2,0,0,0);
	}
	dt2 += dts;  //First whichDay

	dts.SetDateTimeSpan(7,0,0,0);
	for (int i=0;i<whichWeek;i++) {
		dt2 += dts;
	}

	if (dt2 < *this) {
		dt2.MoveToNextMonthlyWeeklyDate(whichWeek,whichDay,day);
		*this = dt2;
		return 0;
	}
	//Maybe last week will become next month.
	if (dt2.GetMonth() != GetMonth()) {
		dts.SetDateTimeSpan(7,0,0,0);
		dt2 -= dts;
		if (dt2.GetMonth() != GetMonth()) {
			dt2 -= dts;
		}
		*this = dt2;
		return 0;
	}
	*this = dt2;

	return 0;
}

int COleDateTimeEx::MoveToNextMonthlyWeeklyDate(int whichWeek,int whichDay,int day)
{
	COleDateTimeEx dt2;
	COleDateTimeSpan dts;
	dt2.SetDate(GetYear(),GetMonth(),28);
	dts.SetDateTimeSpan(5,0,0,0);
	dt2 += dts;
	dt2.SetDate(dt2.GetYear(),dt2.GetMonth(),1);
	COleDateTimeEx dt3 = dt2; // for next month info
	int doW2 = dt2.GetDayOfWeek();
	doW2--;

	if (whichDay < doW2) {
		dts.SetDateTimeSpan(whichDay-doW2+7,0,0,0);
		whichWeek--;
	} else {
		dts.SetDateTimeSpan(whichDay-doW2,0,0,0);
		whichWeek--; //peggy 0614
	}


	dt2 += dts;  //First whichDay

	dts.SetDateTimeSpan(7,0,0,0);
	for (int i=0;i<whichWeek;i++) {
		dt2 += dts;
	}

	if (dt2 < *this) {
		dt2.MoveToNextMonthlyWeeklyDate(whichWeek,whichDay,day);
		*this = dt2;
		return 0;
	}
	if (dt2.GetMonth() != dt3.GetMonth()) {
		dts.SetDateTimeSpan(7,0,0,0);
		dt2 -= dts;
		if (dt2.GetMonth() != dt3.GetMonth()) {
			dt2 -= dts;
		}
		*this = dt2;
		return 0;
	}
	*this = dt2;

	return 0;
}

int COleDateTimeEx::MoveToMonthFirstDate()
{
	SetDate(GetYear(),GetMonth(),1);

	return 0;
}

int COleDateTimeEx::MoveToMonthLastDate()
{
	MoveToNextMonthFirstDate();
	COleDateTimeSpan dts;
	dts.SetDateTimeSpan(1,0,0,0);
	*this -= dts;

	return 0;
}

int COleDateTimeEx::MoveToNextMonthFirstDate()
{
	COleDateTimeSpan dts;
	SetDate(GetYear(),GetMonth(),28);
	dts.SetDateTimeSpan(5,0,0,0);
	*this += dts;
	SetDate(GetYear(),GetMonth(),1);

	return 0;
}

int COleDateTimeEx::MoveToPreMonthFirstDate()
{
	COleDateTimeSpan dts;
	SetDate(GetYear(),GetMonth(),1);
	dts.SetDateTimeSpan(5,0,0,0);
	*this -= dts;
	SetDate(GetYear(),GetMonth(),1);

	return 0;
}

int COleDateTimeEx::MoveToPreMonthLastDate()
{
	COleDateTimeSpan dts;
	SetDate(GetYear(),GetMonth(),1);
	dts.SetDateTimeSpan(1,0,0,0);
	*this -= dts;

	return 0;
}

int COleDateTimeEx::GetWeekOfMonthByWeekDay()
{
	int dayOfWeek = GetDayOfWeek();
	COleDateTimeEx dt = *this;
	dt.MoveToMonthFirstDate();
	int firstDayOfWeek = dt.GetDayOfWeek();
	if (firstDayOfWeek <= dayOfWeek) { //Move to first same day of week //peggy 0614
		dt.MoveToNextDayDate(dayOfWeek-firstDayOfWeek);
	} else {
		dt.MoveToNextDayDate(dayOfWeek-firstDayOfWeek+7);
	}
	COleDateTimeSpan dts = *this - dt;

	return (dts.GetDays() / 7)+1;
}
BOOL COleDateTimeEx::IsMonthLastDay()
{
	COleDateTime temp;
	temp.SetDate(GetYear(),GetMonth()+1,1);
	COleDateTimeSpan dts;
	dts.SetDateTimeSpan(1,0,0,0);
	temp -= dts;

	int nLastDay = temp.GetDay();
	if(GetDay() == nLastDay)
		return TRUE;
	return FALSE;
}
