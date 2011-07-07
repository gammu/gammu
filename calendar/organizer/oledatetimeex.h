// OleDateTimeEx.h: interface for the COleDateTimeEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OLEDATETIMEEX_H__B3C19894_7098_470B_93E6_B9AE439B72EE__INCLUDED_)
#define AFX_OLEDATETIMEEX_H__B3C19894_7098_470B_93E6_B9AE439B72EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "..\stdafx.h"

class COleDateTimeEx : public COleDateTime  
{
public:
	COleDateTimeEx();
	COleDateTimeEx(const COleDateTimeEx& dateSrc): COleDateTime((COleDateTime)dateSrc){}
	COleDateTimeEx(const COleDateTime& dateSrc): COleDateTime(dateSrc){}
	COleDateTimeEx(const VARIANT& varSrc): COleDateTime(varSrc){}
	COleDateTimeEx(DATE dtSrc): COleDateTime(dtSrc) {}

	COleDateTimeEx(time_t timeSrc): COleDateTime(timeSrc){}
	COleDateTimeEx(const SYSTEMTIME& systimeSrc) : COleDateTime(systimeSrc) {}
	COleDateTimeEx(const FILETIME& filetimeSrc) : COleDateTime(filetimeSrc){}

	COleDateTimeEx(int nYear, int nMonth, int nDay,int nHour, int nMin, int nSec)
		: COleDateTime(nYear,nMonth,nDay,nHour,nMin,nSec){}
	COleDateTimeEx(WORD wDosDate, WORD wDosTime) : COleDateTime(wDosDate, wDosTime){}

	virtual ~COleDateTimeEx();

public:
	void SetStatus(DateTimeStatus status) {COleDateTime::SetStatus(status);};
	DateTimeStatus GetStatus() const  {return COleDateTime::GetStatus();};

	BOOL GetAsSystemTime(SYSTEMTIME& sysTime) const  {return COleDateTime::GetAsSystemTime(sysTime);};

	int GetYear() const  {return COleDateTime::GetYear();};
	int GetMonth() const  {return COleDateTime::GetMonth();};       // month of year (1 = Jan)
	int GetDay() const  {return COleDateTime::GetDay();};         // day of month (0-31)
	int GetHour() const  {return COleDateTime::GetHour();};        // hour in day (0-23)
	int GetMinute() const  {return COleDateTime::GetMinute();};      // minute in hour (0-59)
	int GetSecond() const  {return COleDateTime::GetSecond();};      // second in minute (0-59)
	int GetDayOfWeek() const  {return COleDateTime::GetDayOfWeek();};   // 1=Sun, 2=Mon, ..., 7=Sat
	int GetDayOfYear() const  {return COleDateTime::GetDayOfYear();};   // days since start of year, Jan 1 = 1

// Operations
public:
	BOOL IsMonthLastDay();
	int MoveToMonthFirstDate();
	int MoveToMonthLastDate();
	int MoveToNextMonthFirstDate();
	int MoveToPreMonthFirstDate();
	int MoveToPreMonthLastDate();
	int GetWeekOfMonthByWeekDay();
	int MoveToPrevDayDate(int dayCnt);
	int MoveToNextDayDate(int dayCnt);
	int MoveToNextWeekDate(int weekCnt);
	int MoveToNextYearDate(int yearCnt,int day);
	int MoveToNextYearDate(int day);
	int MoveToNextMonthDate(int monthCnt,int day);
	int MoveToNextMonthDate(int day);
	int MoveToFirstMonthlyWeeklyDate(int whichWeek,int whichDay,int day);
	int MoveToNextMonthlyWeeklyDate(int monthCnt,int whichWeek,int whichDay,int day);
	int MoveToNextMonthlyWeeklyDate(int whichWeek,int whichDay,int day);
	const COleDateTimeEx& operator=(const COleDateTime& dateSrc)  {COleDateTime::operator=(dateSrc); return *this;};
	const COleDateTimeEx& operator=(const COleDateTimeEx& dateSrc)  {operator=((COleDateTime)dateSrc); return *this;};
	const COleDateTimeEx& operator=(const VARIANT& varSrc)  {COleDateTime::operator=(varSrc); return *this;};
	const COleDateTimeEx& operator=(DATE dtSrc)  {COleDateTime::operator=(dtSrc); return *this;};

	const COleDateTimeEx& operator=(const time_t& timeSrc)  {COleDateTime::operator=(timeSrc); return *this;};
	const COleDateTimeEx& operator=(const SYSTEMTIME& systimeSrc)  {COleDateTime::operator=(systimeSrc); return *this;};
	const COleDateTimeEx& operator=(const FILETIME& filetimeSrc)  {COleDateTime::operator=(filetimeSrc); return *this;};

	BOOL operator==(const COleDateTime& date) const  {return COleDateTime::operator==(date);};
	BOOL operator!=(const COleDateTime& date) const  {return COleDateTime::operator!=(date);};
	BOOL operator<(const COleDateTime& date) const  {return COleDateTime::operator<(date);};
	BOOL operator>(const COleDateTime& date) const {return COleDateTime::operator>(date);};
	BOOL operator<=(const COleDateTime& date) const  {return COleDateTime::operator<=(date);};
	BOOL operator>=(const COleDateTime& date) const  {return COleDateTime::operator>=(date);};

	BOOL operator==(const COleDateTimeEx& date) const  {return operator==((COleDateTime)date);};
	BOOL operator!=(const COleDateTimeEx& date) const  {return operator!=((COleDateTime)date);};
	BOOL operator<(const COleDateTimeEx& date) const  {return operator<((COleDateTime)date);};
	BOOL operator>(const COleDateTimeEx& date) const {return operator>((COleDateTime)date);};
	BOOL operator<=(const COleDateTimeEx& date) const  {return operator<=((COleDateTime)date);};
	BOOL operator>=(const COleDateTimeEx& date) const  {return operator>=((COleDateTime)date);};

	// DateTime math
	COleDateTimeEx operator+(const COleDateTimeSpan& dateSpan) const  {return COleDateTime::operator+(dateSpan);};
	COleDateTimeEx operator-(const COleDateTimeSpan& dateSpan) const  {return COleDateTime::operator-(dateSpan);};
	const COleDateTimeEx& operator+=(const COleDateTimeSpan dateSpan)  {COleDateTime::operator+=(dateSpan); return *this;};
	const COleDateTimeEx& operator-=(const COleDateTimeSpan dateSpan)  {COleDateTime::operator-=(dateSpan); return *this;};

	// DateTimeSpan math
	COleDateTimeSpan operator-(const COleDateTime& date) const  {return COleDateTime::operator-(date);};
	COleDateTimeSpan operator-(const COleDateTimeEx& date) const  {return operator-((COleDateTime)date);};

	operator DATE() const;

	int SetDateTime(int nYear, int nMonth, int nDay,
		int nHour, int nMin, int nSec)  {return COleDateTime::SetDateTime(nYear, nMonth, nDay,
		nHour, nMin, nSec);};
	int SetDate(int nYear, int nMonth, int nDay)  {return COleDateTime::SetDate(nYear, nMonth, nDay);};
	int SetTime(int nHour, int nMin, int nSec)  {return COleDateTime::SetTime(nHour, nMin, nSec);};
	BOOL ParseDateTime(LPCTSTR lpszDate, DWORD dwFlags = 0,
		LCID lcid = LANG_USER_DEFAULT)  {return COleDateTime::ParseDateTime(lpszDate, dwFlags,lcid);};

	// formatting
	CString Format(DWORD dwFlags = 0, LCID lcid = LANG_USER_DEFAULT) const  {return COleDateTime::Format(dwFlags, lcid);};
	CString Format(LPCTSTR lpszFormat) const  {return COleDateTime::Format(lpszFormat);};
	CString Format(UINT nFormatID) const  {return COleDateTime::Format(nFormatID);};

};

#endif // !defined(AFX_OLEDATETIMEEX_H__B3C19894_7098_470B_93E6_B9AE439B72EE__INCLUDED_)
