// header file 

#ifndef _CCAMMSync_
#define _CCAMMSync_

#include "..\stdafx.h"
#include "CAMMSyncInclude.h"
#include "..\Organizer\ScheduleInfo.h"
#include "..\Calendar.h"

//CString GuidToString(GUID guid);
//void StringToGuid(CString szStr, GUID& inputGUID);
typedef struct _NoteTypeSupportInfo
{
	DWORD dwSupportFlag;
	//0x00000001 text1
	//0x00000002 text2
	//0x00000004 startDate
	//0x00000008 startTime
	//0x00000010 endDate
	//0x00000020 endTime
	//0x00000040 alarmDate
	//0x00000080 alarmTime
	//0x00000100 alarmTone
	//0x00000200 repeat
	//0x00000400 repeatEndDate
	//0x00000800 alarmLeadTime



	TCHAR stTypeID[MAX_PATH];
	TCHAR stText1ID[MAX_PATH];
	TCHAR stText2ID[MAX_PATH];
	DWORD dwAlarmLeadTimeType;
	DWORD dwRepeatType;
	DWORD dwEndTimeIntervalType;
	DWORD dwAlarmTimeIntervalType;
	int   nEndTimeIntervalMax;
	int   nAlarmTimeIntervalMax;
	int	  nNodeType;
	BOOL  bPhoneNoText2;
	int   nRepeatdefaulType;
	TCHAR  szIconfileName[MAX_PATH];
	int nDefaultAlarmcheck;
	TCHAR stText3ID[MAX_PATH];

	int   nAlarmTimeIntervalMax_Hour;
	int   nAlarmTimeIntervalMax_Min;
	int   nAlarmTimeIntervalMax_Sec;
	int   nAlarmTimeIntervalMax_Day;
	int   nAlarmTimeIntervalMax_Week;
	int   nAlarmTimeIntervalMax_Month;
	int   nAlarmTimeIntervalMax_Year;
	int   bDisableEndDateWhenRepeat;
	int   nRepeatIntervalYear;
}NoteTypeSupportInfo;

static CArray<int,int> m_ArrayofSupportNoteType;
static int             m_SizeofSupportNoteType;

class CCAMMSync
{
	
// The actual APIs are accessed through wrapper classes,the advantage is that 
// if we need to support other vendors the underlaying APIs in the wrapper will change
// thus the real API and program code will not change
private:
	HINSTANCE m_hMODLL;

public:
	void xErrorHandling(int iErr);

	CCAMMSync();
	~CCAMMSync();
	BOOL CheckMobileSetDateTime();
	BOOL   m_bOpenDriverFinal;

//	BOOL xLoadLibrary();
//	void xFreeLibrary();
	//nType can be MO_ADR or MO_NOTE or MO_SCH
//	CAMMError GetRecordCount(int nType, CString dbPath,int &recordCount);
//	CAMMError LoadContact(ContactsRecord record[],CString csDBPath,int &nSize);
//	CAMMError SaveContact(ContactsRecord record[],CString csDBPath,int nSize);

//	CAMMError LoadNotes(NotesRecord record[],CString csDBPath,int &nSize);
//	CAMMError SaveNotes(NotesRecord record[],CString csDBPath,int nSize);

//	CAMMError LoadSchedule(SchedulesRecord record[],CString csDBPath,int &nSize);
//	CAMMError SaveSchedule(SchedulesRecord record[],CString csDBPath,int nSize);
//	CAMMError IsValidOrgnizerFile(CString dbPath);
	/*
	_MoOpenMyOrganizer			pfn_MoOpenMyOrganizer;
	_MoCloseMyOrganizer			pfn_MoCloseMyOrganizer;
	_MoOpenFile					pfn_MoOpenFile;
	_MoCloseFile				pfn_MoCloseFile;
	_MoOpenDatabase				pfn_MoOpenDatabase;
	_MoCloseDatabase			pfn_MoCloseDatabase;
	_MoCreateRecord				pfn_MoCreateRecord;
	_MoGetRecordFieldNameByID	pfn_MoGetRecordFieldNameByID;
	_MoSetRecordField			pfn_MoSetRecordField;
	_MoGetRecordField			pfn_MoGetRecordField;
	_MoCloseRecord				pfn_MoCloseRecord;
	_MoGetRecordCount			pfn_MoGetRecordCount;
	_MoSetGeneralRecordRecurInfo pfnMoSetGeneralRecordRecurInfo;
	_MoGetGeneralRecordRecurInfo pfnMoGetGeneralRecordRecurInfo;
*/
    //For CAMMagic Mobie 2.0
	//HMODULE              m_hMobileDll;
//	HWND                 m_hWnd;
	bool                 m_bOpened;
	//static Calendar_Data_Struct m_Calendar_Data_Strc;
    SchedulesRecord* m_pRecord;
	int m_nRecordCount;

//    void       OnGetMobileCal();
	static int ConnectStatusCallBack(int State);//CallBack Function
    int        GetCalendarCount();
	bool       LoadandOpenCalendarDriver();
	bool       CloseCalendarDriver();
	bool       InitCalendarDriver();
	void       FreeCalendarDriver();
	CAMMError  ConvertPCDataToMobile(CScheduleInfo Record,Calendar_Data_Struct* pCalendarData,int& nErrorCode);

    CAMMError  ConvertPCDataToMobile(CScheduleInfo Record,int& nErrorCode);
    CAMMError  ConvertMoblieDataToPC(Calendar_Data_Struct CalendarDataFromMobile, SchedulesRecord* record, int& nErrorCode);
    CAMMError  LoadCalendarfromMobile(SchedulesRecord record[], int nRecordCount);
    //bool       LoadDriverandAddCalendar(CScheduleInfo Record);
    //static int AddCalendarCallBack(int State);//CallBack Function
    //static int AddCalendar(void);
    int AddCalendar(CScheduleInfo& Record);
    int AddCalendar(CScheduleInfo& Record,Control_Type& nType,SchedulesRecord* record = NULL);
    int EditCalendar(CScheduleInfo& Record);
    int DeleteCalendar(CScheduleInfo& Record,int Control);
    bool GetCalendarSupport(int& nSupportNum,  CArray<NoteTypeSupportInfo,NoteTypeSupportInfo>& ArraySupportType);
    static bool GetCalendarSupport();
    void GetCalendarSupport(CalendarSupportInfo& SupportInfo);
    int WrapperGSMDateTime(COleDateTimeEx dtStartDate, CScheduleTimeInfo st, GSM_DateTime& gsmDateTime);
    int WrapperGSMDateTime(COleDateTimeEx dtStartDate, GSM_DateTime& gsmDateTime);
	void GSMTimetoDATE(GSM_DateTime gsmtime,COleDateTime& date);
	void DATEtoGSMTime(COleDateTime date,GSM_DateTime& gsmtime);
	BOOL GetSupportFlag(LPCTSTR szProfile,LPCTSTR szSection,LPCTSTR szDefaultSection,LPCTSTR szKey, DWORD& dwFlag);
	BOOL GetNoteTypeSupportInfo(int nNoteType,NoteTypeSupportInfo *pSupportInfo);
	//DLL CONTRUCT
	// A Condition to Free the AnwMobile Library 
    void        FreeLibrary(HMODULE m_hDriver);

private:
//	CAMMError xGetRecordCount(int nType, CString szProductPath, CString dbPath,int &recordCount);
//	CAMMError xSaveContact(ContactsRecord record[],CString szProductPath,CString csDBPath,int size);
//	CAMMError xSaveNotes(NotesRecord record[],CString szProductPath,CString csDBPath,int size);
//	CAMMError xSaveSchedule(SchedulesRecord record[],CString szProductPath,CString csDBPath,int size);

//	CAMMError xCheckFileExisted(CString csDBPath);
//	CAMMError xGetSystemTempPath(CString &szTempPath);
//	CAMMError xGetProductPath(CString &szTempPath,CString csDBPath);
//	CAMMError xCreateEmptyNewFile(CString csDBPath);
//	CAMMError xRemoveOrganizer(int nType,CString szProductPath,CString csDBPath);
//	CAMMError xCopyContact(CString szProductPath,CString csSrcDBPath,CString csTgtDBPath);
//	CAMMError xCopyNote(CString szProductPath,CString csSrcDBPath,CString csTgtDBPath);
//	CAMMError xCopySchedule(CString szProductPath,CString csSrcDBPath,CString csTgtDBPath);

//	CAMMError xLoadContact(ContactsRecord record[],CString szProductPath,CString csDBPath,int &nSize);
//	CAMMError xLoadNotes(NotesRecord record[],CString szProductPath,CString csDBPath,int &nSize);
//	CAMMError xLoadSchedule(SchedulesRecord record[],CString szProductPath,CString csDBPath,int &nSize);

    //For CAMMagic Mobie 2.0
//    CAMMError xLoadCalendarfromMobile(SchedulesRecord record[], int nRecordCount);
};

#endif
