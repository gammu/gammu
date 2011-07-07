// ScheduleInfo.h: interface for the CScheduleInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCHEDULEINFO_H__8C843040_515C_4C6A_A005_02A3B22D3C17__INCLUDED_)
#define AFX_SCHEDULEINFO_H__8C843040_515C_4C6A_A005_02A3B22D3C17__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "OrganizeDeclaration.h"
#include "ScheduleTimeInfo.h"

class CScheduleInfo  
{
public:
	CScheduleInfo();
	virtual ~CScheduleInfo();

public:
	void MergeAndValidateSchedule(CScheduleInfo si);
	CString GetClipboardString();
	void SetScheduleValue(int idx,CString str);
	BOOL ParseClipboardString(CString cpStr);
	const CScheduleInfo& operator =(CScheduleInfo &si);
	void GetFirstMonthlyWeeklyDate(int whichWeek,int whichDay,COleDateTimeEx &dt,int day);
	void GetNextMonthlyWeeklyDate(int whichWeek,int whichDay,COleDateTimeEx &dt,int day);
	BOOL IsStartDateFirstDateToDisplay();
	int GetFirstExtendDate(COleDateTimeEx &dt,COleDateTimeEx &dtStop,int year,int month,int day,COleDateTime StartEndDateSpin);
	int GetNextExtendDate(COleDateTimeEx &dt,COleDateTimeEx &dtStop,int day,COleDateTime StartEndDateSpin);
	CScheduleInfo(CScheduleInfo &si);
	void SetScheduleInfoData(const CScheduleInfo &si);
//	CString getAlarmLeadtime();
//	void SetAlarmLeadtime(CString leadtime);
	CString GetStartTimeString();
	CString GetStopTimeString();
	CString GetRealStartTimeString();
	CString GetRealStopTimeString();
	CString GetStartDateString();
	CString GetStopDateString();
	CString toString();
//	BOOL ParseString(CString inStr);
	void Reset();
	int GetStopRow();
	void SetData(SchedulesRecord &sr);
	void ParseDateTimeString(CString &cs,int &year,int &month,int &day,
		                   int &hour,int &min,int &sec);
	void ParseDateString(CString &cs,int &year,int &month,int &day);
	BOOL ParseTimeString(CString &inStr,int &hour,int &min,int &sec);

	CString GuidToString(GUID guid);
	void StringToGuid(CString szStr, GUID& inputGUID);

//    CString SetAlarmLeadDatetime();
	//
//    CString getAlarmLeadtime(int nIndexAlarmLeadtime);
	BOOL IsDateExcessEndDate(COleDateTimeEx dt);


	//Calendar
    void DumpScheduleConntent();

public: //member
	void UpdateData(CScheduleInfo diffSi,DWORD dwUpdateFlag);
	DWORD GetDifferFlag(CScheduleInfo diffSi);
	DWORD GetAlarmBeforeMin(COleDateTime strStartTime , COleDateTime strAlarmtime);
	int nRepeatId;
	COleDateTimeEx dtStartDate;
	int rowIndex;
	int n30MinBlockCnt;
	BOOL bScheduleStart;
	CScheduleTimeInfo hiStartTime;
	CScheduleTimeInfo hiStopTime;
	CScheduleTimeInfo hiRealStartTime;
	CScheduleTimeInfo hiRealStopTime;

	//	CScheduleTimeInfo hiStopTime;
	CString csTitle;
	CString csMemo;

	BOOL	haveRepeatUntil;  //m_bRepeatUntil
	ScheduleDate	utilWhichDate;    //m_UntilWhichDate
	CString telnumber; //telephone number to be reminded on : m_AlarmPhoneNo
	BOOL haveAlarm; //true or false  : m_bAlarm
//	UINT alarmLeadtime; //Similar to Reminder option of outlook. must be in minutes. : m_AlarmLeadtime

	BOOL originalData;
	BOOL haveRepeat; //true or false : m_bRepeat
	INT repeatType; //Daily , weekly, Monthly, MonthlyWeekly and Yearly : m_RepeatType
	UINT repeatFrequency; // every 1 day/week/month/year : m_RepeatFrequency
	UINT rfWhichWeek; //First , Second, Third, Fourth or Last week : m_RepeatWhichWeek
	UINT rfWhichDay; //Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday : m_WeekDay
	int nDayofWeekMask;
	CString csReasonForReply;
	GUID OrgId;
	BOOL bEndDateSameToStartDate; //ธ๓คั

	//Calendar
//	UINT uiIndex;//Index assigned Mobile.
	char szuiIndex[100];
	CString cuLocation; //Location of the shcedule
	UINT uiNodeType;//NodeType of the shcedule
	UINT uiPrivate;//int uiPrivate
	BOOL bhaveTone;
	COleDateTimeEx dtBirthDate;
	COleDateTimeSpan dtAlarmLeadTimeSpan;
	COleDateTime   dtAlarmDateTime;
	COleDateTime	dtWeekMaskStartday;
  //  CString   cuAlarmLeadDatetime;

	COleDateTimeEx   dtStopDate;
};
BOOL operator ==(const CScheduleInfo& info1, const CScheduleInfo& info2);

#endif // !defined(AFX_SCHEDULEINFO_H__8C843040_515C_4C6A_A005_02A3B22D3C17__INCLUDED_)
