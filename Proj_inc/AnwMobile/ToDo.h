#ifndef _ToDo_
#define _ToDo_
#include "GSM_DATATYPE.h"
#include "calendar.h"
#include "gsmcal.h"

typedef struct
{
  GSM_DateTime Start_DateTime;
  GSM_DateTime End_DateTime;
  GSM_DateTime Alarm_DateTime;
  GSM_DateTime Completed_DateTime;
  bool haveTone;
  int NodeType;
  TCHAR Text[514];
  int Private;
  int ContactID;
  char Index[100];
  int RecurrenceType ; // 0 : 不循環 ; 1: Daily ; 2: Weekly ;3:Monthly ; 4:Monthly by week 5:Yearly
  int RecurrenceFrequency;  // 每幾天/ 每幾週 /每幾月/….
  GSM_DateTime Repeat_StartDate;
  GSM_DateTime Repeat_EndDate;
  int Priority	; // 1:High 2:Medium 3:Low
  int Completed;  //0: not completed 1: Completed
  
  TCHAR szDescription[514];
  long Repeat_DayOfWeekMask ; // 0 : only start date ;1:SUN 2:MON 4:TUE 8:WED 16 :THU 32:FRI 64 SAT

} ToDo_Data_Struct;



typedef struct
{
	int		SupportNodeNumber;
	int		TotalToDo;
	int     Title_Boundry;
	int     Location_Boundry;
	int     Description_Boundry;
	DWORD   NodeFormat;	
			//ToDo	GSM_CAL_MEETING							0x00000001
			//Call	GSM_CAL_CALL							0x00000002
			//GSM_CAL_REMINDER								0x00000004
			//GSM_CAL_MEMO									0x00000008
			//GSM_CAL_TRAVEL								0x00000010								
	int  CharType;   
}ToDoSupportInfo;

//#define SchedulesRecord  Calendar_Data_Struct 

void ToDo_Data_Struct2Entry(GSM_ToDoEntry* Entry,ToDo_Data_Struct *Schedule_Data_Strc);
void Entry2ToDo_Data_Struct(GSM_ToDoEntry* Entry,ToDo_Data_Struct *Schedule_Data_Strc);


int WINAPI OpenToDo(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI, int (*ConnectStatusCallBack)(int State) );
int WINAPI GetMobileToDoInfo(Schedule_Tal_Num *Schedule_Tal_Num);
int WINAPI GetToDoStartData(int NeedCount, Calendar_Data_Struct *Schedule_Data_Strc, int &RealCount);
int WINAPI GetToDoNextData(int NeedCount, Calendar_Data_Struct *Schedule_Data_Strc, int &RealCount);
int WINAPI AddToDoData(Calendar_Data_Struct *Schedule_Data_Strc);
int WINAPI DeleteToDoData(int Index, Calendar_Data_Struct *Schedule_Data_Strc);
int WINAPI CloseToDo(void);
int WINAPI InitToDo(void);
int WINAPI TerminateToDo(void);
int WINAPI GetToDoSupport(ToDoSupportInfo *SupportInfo);
int WINAPI InitialToDoStatus(Schedule_Tal_Num *Schedule_Tal_Num);
int WINAPI GetToDoPriority(GSM_ToDo_Priority gsmPriority);
GSM_ToDo_Priority WINAPI GetToDoGSMPriority(int nPriority);
int WINAPI InitialToDoStatusEx(Schedule_Tal_Num *Schedule_Tal_Num,int (*pGetStatusfn)(int nCur,int nTotal));
int WINAPI DeleteToDoDataEx(int Index, Calendar_Data_Struct *Schedule_Data_Strc,char* pszOriindex);

typedef int (WINAPI* anwOpenToDo)(int MobileCompany , TCHAR *MobileID , TCHAR *ConnectMode , TCHAR *ConnectPortName ,TCHAR *IMEI, int (*ConnectStatusCallBack)(int State) );
typedef int (WINAPI* anwCloseToDo)(void);
typedef int (WINAPI* anwGetMobileToDoInfo)(Schedule_Tal_Num *Schedule_Tal_Num);
typedef int (WINAPI* anwGetToDoStartData)(int NeedCount, ToDo_Data_Struct *Schedule_Data_Strc, int &RealCount);
typedef int (WINAPI* anwGetToDoNextData)(int NeedCount, ToDo_Data_Struct *Schedule_Data_Strc, int &RealCount);
typedef int (WINAPI* anwAddToDoData)(ToDo_Data_Struct *Schedule_Data_Strc,int control);
typedef int (WINAPI* anwDeleteToDoData)(char* pszIndex, ToDo_Data_Struct *Schedule_Data_Strc,int control);
typedef int (WINAPI* anwInitToDo)(void);
typedef int (WINAPI* anwTerminateToDo)(void);
typedef int (WINAPI* anwGetToDoSupport)(ToDoSupportInfo *SupportInfo);
typedef int (WINAPI* anwInitialToDoStatus)(Schedule_Tal_Num *Schedule_Tal_Num);
typedef int (WINAPI* anwGetToDoOriIndex)(char*,char*);
typedef int (WINAPI* anwInitialToDoStatusEx)(Schedule_Tal_Num *Schedule_Tal_Num,int (*pGetStatusfn)(int nCur,int nTotal));
typedef int (WINAPI* anwDeleteToDoDataEx)(char* pszIndex, ToDo_Data_Struct *Schedule_Data_Strc,int control,char *pszOriIndex);

extern anwOpenToDo				ANWOpenToDo;
extern anwCloseToDo				ANWCloseToDo;
extern anwGetMobileToDoInfo		ANWGetMobileToDoInfo;
extern anwGetToDoStartData		ANWGetToDoStartData;
extern anwGetToDoNextData		ANWGetToDoNextData;
extern anwAddToDoData			ANWAddToDoData;
extern anwDeleteToDoData		ANWDeleteToDoData;
extern anwInitToDo			ANWInitToDo;
extern anwTerminateToDo		ANWTerminateToDo;
extern anwGetToDoSupport		ANWGetToDoSupport;
extern anwInitialToDoStatus			ANWInitialToDoStatus;
extern anwGetToDoOriIndex ANWGetToDoOriIndex;
extern anwInitialToDoStatusEx			ANWInitialToDoStatusEx;
extern anwDeleteToDoDataEx		ANWDeleteToDoDataEx;
#endif