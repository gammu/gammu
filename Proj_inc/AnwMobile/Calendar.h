#ifndef _Calendar_
#define _Calendar_
#include "GSM_DATATYPE.h"

typedef struct
{
  GSM_DateTime Start_DateTime;
  GSM_DateTime End_DateTime;
  GSM_DateTime Alarm_DateTime;
  //GSM_DateTime Silent_Alarm_DateTime;
  bool haveTone;
  int NodeType;
  TCHAR Location[514];
  //char Phone[514];
  TCHAR Text[514];
  //char RingName[514];
  //unsigned int Recurrence;		   // mingfa
  int Private;
  int ContactID;
 // int Index;
  char szIndex[100];
  int RecurrenceType ; // 0 : 不循環 ; 1: Daily ; 2: Weekly ;3:Monthly ; 4:Monthly by week 5:Yearly
  int RecurrenceFrequency;  // 每幾天/ 每幾週 /每幾月/….
  //int Repeat_DayOfWeek;
  //int Repeat_Day;
  //int Repeat_WeekOfMonth;
  //int Repeat_Month;
  //int Repeat_Frequency;
  GSM_DateTime Repeat_StartDate;
  GSM_DateTime Repeat_EndDate;
  //new for Nokia S60
  TCHAR szDescription[514];
  long Repeat_DayOfWeekMask ; // 0 : only start date ;1:SUN 2:MON 4:TUE 8:WED 16 :THU 32:FRI 64 SAT
} Calendar_Data_Struct;


typedef struct 
{
	short  title;		// HighByte : Size limitation ; Low byte Mobile On :=0x01 ( bit flag )
	short  starttime;	// HighByte : Size limitation ; Low byte Mobile On :=0x01 ; ( bit flag )
	short  endtime;
} ScheduleStrc_Flag;

typedef struct
{
	

} MobilScheduleStrc;

typedef struct
{ 
  short ScheduleUsedNum;
} Schedule_Tal_Num;

typedef struct
{
	int		SupportNodeNumber;
	int		TotalCalendar;
	int     Title_Boundry;
	int     Description_Boundry;
	int     Location_Boundry;
	DWORD   NodeFormat;	
			// Reminder or Date								0x00000001
			//Call											0x00000002
			//Meeting										0x00000004
			//Birthday or Anniversary or Special Occasion	0x00000008
			// Memo or Miscellaneous						0x00000010
			//Travel										0x00000020
			//Vacation										0x00000040
			//Training - Athletism							0x00000080
			//Training - Ball Games							0x00000100
			//Training - Cycling							0x00000200
			//Training - Budo								0x00000400
			//Training - Dance								0x00000800
			//Training - Extreme Sports						0x00001000
			//Training - Football							0x00002000
			//Training - Golf								0x00004000
			//Training - Gym								0x00008000
			//Training - Horse Race							0x00010000
			//Training - Hockey								0x00020000
			//Training - Races								0x00040000
			//Training - Rugby								0x00080000
			//Training - Sailing							0x00100000
			//Training - Street Games						0x00200000
			//Training - Swimming							0x00400000
			//Training - Tennis								0x00800000
			//Training - Travels							0x01000000
			//Training - Winter Games						0x02000000
			//Alarm											0x04000000
			//Alarm repeating each day.						0x08000000
			// Low											0x10000000	
			// Normal										0x20000000	
			// High											0x40000000	
	int  CharType;   // added by mingfa for SE_Z600
}CalendarSupportInfo;

//#define SchedulesRecord  Calendar_Data_Struct 

void Cal_Data_Struct2Entry(GSM_CalendarEntry* Entry,Calendar_Data_Struct *Schedule_Data_Strc);
void Entry2Cal_Data_Struct(GSM_CalendarEntry* Entry,Calendar_Data_Struct *Schedule_Data_Strc);


int WINAPI OpenCalendar(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI, int (*ConnectStatusCallBack)(int State) );
int WINAPI GetMobileScheduleStrc(MobilScheduleStrc  *MobilScheduleFlag);
int WINAPI GetMobileScheduleInfo(Schedule_Tal_Num *Schedule_Tal_Num);
int WINAPI GetScheduleStartData(int NeedCount, Calendar_Data_Struct *Schedule_Data_Strc, int &RealCount);
int WINAPI GetScheduleNextData(int NeedCount, Calendar_Data_Struct *Schedule_Data_Strc, int &RealCount);
int WINAPI AddScheduleData(Calendar_Data_Struct *Schedule_Data_Strc);
int WINAPI DeleteScheduleData(char* szLocation, Calendar_Data_Struct *Schedule_Data_Strc);
int WINAPI DeleteAllScheduleData(Calendar_Data_Struct *Schedule_Data_Strc);
int WINAPI CloseCalendar(void);
int WINAPI InitCalendar(void);
int WINAPI TerminateCalendar(void);
int WINAPI GetCalendarSupport(CalendarSupportInfo *SupportInfo);
int WINAPI GetMobileDateTime(GSM_DateTime *DateTime);
int WINAPI DeleteScheduleDataEx(char* szLocation, Calendar_Data_Struct *Schedule_Data_Strc,char* pszOriIndex);

typedef int (WINAPI* anwOpenCalendar)(int MobileCompany , TCHAR *MobileID , TCHAR *ConnectMode , TCHAR *ConnectPortName ,TCHAR *IMEI, int (*ConnectStatusCallBack)(int State) );
typedef int (WINAPI* anwCloseCalendar)(void);
typedef int (WINAPI* anwGetMobileScheduleStrc)(MobilScheduleStrc  *MobilScheduleFlag);
typedef int (WINAPI* anwGetMobileScheduleInfo)(Schedule_Tal_Num *Schedule_Tal_Num);
typedef int (WINAPI* anwGetScheduleStartData)(int NeedCount, Calendar_Data_Struct *Schedule_Data_Strc, int &RealCount);
typedef int (WINAPI* anwGetScheduleNextData)(int NeedCount, Calendar_Data_Struct *Schedule_Data_Strc, int &RealCount);
typedef int (WINAPI* anwAddScheduleData)(Calendar_Data_Struct *Schedule_Data_Strc,int control);
typedef int (WINAPI* anwDeleteScheduleData)(char* szLocation, Calendar_Data_Struct *Schedule_Data_Strc,int control);
typedef int (WINAPI* anwDeleteScheduleDataEx)(char* szLocation, Calendar_Data_Struct *Schedule_Data_Strc,int control,char *pszOriIndex);
typedef int (WINAPI* anwDeleteAllScheduleData)(Calendar_Data_Struct *Schedule_Data_Str);
typedef int (WINAPI* anwInitCalendar)(void);
typedef int (WINAPI* anwTerminateCalendar)(void);
typedef int (WINAPI* anwGetCalendarSupport)(CalendarSupportInfo *SupportInfo);
typedef int (WINAPI* anwGetMobileDateTime)(GSM_DateTime *DateTime);
typedef int (WINAPI* anwInitialCalendarStatus)(Schedule_Tal_Num *Schedule_Tal_Num);
typedef int  (WINAPI*anwGetCalendarOriIndex)(char* ,char*);
typedef int (WINAPI* anwInitialCalendarStatusEx)(Schedule_Tal_Num *Schedule_Tal_Num,int (*pGetStatusfn)(int nCur,int nTotal));
typedef int (WINAPI* anwInitialCalendarAndToDoStatusEx)(Schedule_Tal_Num *pCalNum,Schedule_Tal_Num *ToDo_Tal_Num,int (*pGetStatusfn)(int nCur,int nTotal));


extern anwOpenCalendar				ANWOpenCalendar;
extern anwCloseCalendar				ANWCloseCalendar;
extern anwGetMobileScheduleStrc		ANWGetMobileScheduleStrc;
extern anwGetMobileScheduleInfo		ANWGetMobileScheduleInfo;
extern anwGetScheduleStartData		ANWGetScheduleStartData;
extern anwGetScheduleNextData		ANWGetScheduleNextData;
extern anwAddScheduleData			ANWAddScheduleData;
extern anwDeleteScheduleData		ANWDeleteScheduleData;
extern anwDeleteScheduleDataEx		ANWDeleteScheduleDataEx;
extern anwDeleteAllScheduleData		ANWDeleteAllScheduleData;
extern anwInitCalendar				ANWInitCalendar;
extern anwTerminateCalendar			ANWTerminateCalendar;
extern anwGetCalendarSupport		ANWGetCalendarSupport;
extern anwGetMobileDateTime			ANWGetMobileDateTime;
extern anwInitialCalendarStatus			ANWInitialCalendarStatus;
extern anwGetCalendarOriIndex ANWGetCalendarOriIndex;
extern anwInitialCalendarStatusEx			ANWInitialCalendarStatusEx;
extern anwInitialCalendarAndToDoStatusEx			ANWInitialCalendarAndToDoStatusEx;
#endif