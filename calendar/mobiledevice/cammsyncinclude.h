#if !defined(AFX_CCAMMSYNCINCLUDE_H__F3DF53D9_6EAF_44BF_A2B0_AD9452E695BD__INCLUDED_)
#define AFX_CCAMMSYNCINCLUDE_H__F3DF53D9_6EAF_44BF_A2B0_AD9452E695BD__INCLUDED_

#include "..\stdafx.h"
//#include "..\OKWAP\Orgnizer\inc\MoExport.h"
//#include "..\OKWAP\Orgnizer\inc\SYRPT.H"

//#define OKWAPMO "KMoSync65.dll"

//Sanjeev 021203
#define SupportFieldNum 14
#define SupportFieldNumNotes 3
#define SupportFieldNumSch 10

#define PCID_MAX_LEN 132
#define SOUND_MAX_LEN 2048
//Sanjeev 021203

#define	LASTNAME_MAX_LEN 32
#define	FIRSTNAME_MAX_LEN 32
#define	COMPANNYNAME_MAX_LEN 56
#define	EMAIL_MAX_LEN 128
#define	NOTES_MAX_LEN 512
#define	PHONE_MAX_LEN 20
#define	CATEGORY_MAX_LEN 20

#define TITLE_MAX_LEN 50
#define MEMO_MAX_LEN 512

#define FIELD_BUFFER_LEN 1024

#define SCH_TIMESTART_MAX_LEN 32
#define SCH_TIMEEND_MAX_LEN 32
#define SCH_TITLE_MAX_LEN 52
#define SCH_MEMO_MAX_LEN 512
#define SCH_TEL_MAX_LEN 512
#define SCH_HAVEALARM_MAX_LEN 12
#define SCH_TIME_MAX_LEN 32
#define SCH_HAVEREPEAT_MAX_LEN 12
#define SCH_MAX_LEN 52
//Comment by Peter 2003/10/29 for changing DAILY from 2 to 0
//typedef enum {DAILY=2, WEEKLY, MONTHLY, WEEKLYMONTHLY, YEARLY} RepeatType;
typedef enum {NOREPEAT=-1, DAILY=0, WEEKLY=1, TWOWEEKLY, MONTHLY, YEARLY, WEEKLYMONTHLY,WEEKDAYS,WEEKEND} RepeatType;

typedef enum {FIRST=0, SECOND, THIRD, FOURTH, LAST} WHICHWEEK;  //used only if WEEKLYMONTHLY Repeat Type
typedef enum {SUNDAY, MONDAY, TUESDAY, WEDNESSDAY, THURSDAY, FRIDAY, SATURDAY} WHICHDAY;  //used only if WEEKLYMONTHLY 

//Calendar, NONO [2004_1105]
//typedef enum {REMIND=1, CALL, MEETING, BIRTHDAY, MEMO} NOTETYPE;//Calendar

#define CAL_LIMIT_OF_NOTETYPE_MOBILESUPPORT 5
/////////////////////////////////////////////////////////////////////////////////////////////////
// Contacts
// {7A2CA26D-371D-427d-BEC9-8924AE265415}
static const GUID IID_NONE = 
{ 0x0, 0x0, 0x0, { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } };
CString GuidToString(GUID guid);
void StringToGuid(CString szStr, GUID& inputGUID);

typedef struct _ContactsRecord
{
	CString lastname;
	CString firstname;
	CString companyname;
	CString email;
	CString notes;
	CString mobilephone;
	CString familyphone;
	CString officephone;
	CString bpnum; //Pager
	CString faxnum;
	CString category;
	CString sound;
	CString qq;
//Initialization
	GUID pcid;
	CString csPcid;
	void SetPCID(GUID id)
	{
		pcid = id;
		csPcid = CString(GuidToString(id));
	}
	GUID GetPCID(void)
	{
		return pcid;
	}
	void SetSound(CString _sound)
	{
		sound = _sound;
	}
	CString GetSound(void)
	{
		return sound;
	}
	void SetRecord(CString _fname, 
				CString _lname,
				CString _category,
				CString _cname, 
				CString _mphone,
				CString _fphone,
				CString _ophone,
				CString _bpnum,
				CString _faxnum,
				CString _email,
				CString _notes,
				CString _qq
				)
	{
		lastname = _lname;
		firstname = _fname;
		companyname = _cname;
		email = _email;
		notes = _notes;
		mobilephone = _mphone;
		familyphone = _fphone;
		officephone = _ophone;
		bpnum = _bpnum; //Pager
		faxnum = _faxnum;
		category = _category;
		qq = _qq;
	}


	_ContactsRecord()
	{
		lastname = _T("");
		firstname = _T("");
		companyname = _T("");
		email = _T("");
		notes = _T("");
		mobilephone = _T("");
		familyphone = _T("");
		officephone = _T("");
		bpnum = _T(""); //Pager
		faxnum = _T("");
		category = _T("");
		pcid = IID_NONE;
		csPcid = _T("");
		sound = _T("");
		qq = _T("");
	}

	~_ContactsRecord()
	{
		lastname = _T("");
		firstname = _T("");
		companyname = _T("");
		email = _T("");
		notes = _T("");
		mobilephone = _T("");
		familyphone = _T("");
		officephone = _T("");
		bpnum = _T(""); //Pager
		faxnum = _T("");
		category = _T("");
		pcid = IID_NONE;
		csPcid = _T("");
		sound = _T("");
		qq = _T("");

	}

	_ContactsRecord *GetRecord()
	{
		return this;
	}

}ContactsRecord;

/*
typedef struct _ContactsRecordWrapper
{
	WORD * InputField[SupportFieldNum];
	UINT InputFieldLen[SupportFieldNum];
	BYTE OutputFieldId[SupportFieldNum];
// Initialization
	_ContactsRecordWrapper()
	{
		
	}

	void PrepareRecord(ContactsRecord &Record)
	{
		if (Record.pcid==IID_NONE) {
			InputField[0] = NULL;
		} else { 
			InputField[0] = Record.csPcid;
		}
		InputField[1] = Record.lastname;
		InputField[2] = Record.firstname;
		InputField[3] = Record.companyname;
		InputField[4] = Record.email;
		InputField[5] = Record.notes;
		InputField[6] = Record.mobilephone;
		InputField[7] = Record.familyphone;
		InputField[8] = Record.officephone;
		InputField[9] = Record.bpnum;
		InputField[10] = Record.faxnum;
		InputField[11]= Record.category;
		InputField[12]= Record.sound;
		InputField[13]= Record.qq;
		
	}
	void Init()
	{
/*		InputFieldLen[0] = PCID_MAX_LEN;
		InputFieldLen[1] = LASTNAME_MAX_LEN;
		InputFieldLen[2] = FIRSTNAME_MAX_LEN;
		InputFieldLen[3] = COMPANNYNAME_MAX_LEN;
		InputFieldLen[4] = EMAIL_MAX_LEN;
		InputFieldLen[5] = NOTES_MAX_LEN;
		InputFieldLen[6] = PHONE_MAX_LEN;
		InputFieldLen[7] = PHONE_MAX_LEN;
		InputFieldLen[8] = PHONE_MAX_LEN;
		InputFieldLen[9] = PHONE_MAX_LEN;
		InputFieldLen[10] = PHONE_MAX_LEN;
		InputFieldLen[11]= CATEGORY_MAX_LEN;
		InputFieldLen[12]= SOUND_MAX_LEN;
		InputFieldLen[13] = PHONE_MAX_LEN;

		
		OutputFieldId[0] = MO_ADR66_PCID;
		OutputFieldId[1] = MO_ADR66_LNAME;
		OutputFieldId[2] = MO_ADR66_FNAME;
		OutputFieldId[3] = MO_ADR66_COMPANYNAME;
		OutputFieldId[4] = MO_ADR66_EMAIL;
		OutputFieldId[5] = MO_ADR66_MEMO;		
		OutputFieldId[6] = MO_ADR66_MOBILENUM;
		OutputFieldId[7] = MO_ADR66_HOMENUM;
		OutputFieldId[8] = MO_ADR66_OFFICENUM;
		OutputFieldId[9] = MO_ADR66_BPNUM;
		OutputFieldId[10] = MO_ADR66_FAXNUM;
		OutputFieldId[11]= MO_ADR66_CATEGORY;
		OutputFieldId[12]= MO_ADR66_SOUND;
		OutputFieldId[13]= MO_ADR65_QQ; //change this to QQ later on
*/
/*	}
	void GetRecord(ContactsRecord *Record)
	{
		Record->csPcid	 = InputField[0];
		Record->lastname = InputField[1];
		Record->firstname=InputField[2];
		Record->companyname=InputField[3];
		Record->email=InputField[4];
		Record->notes=InputField[5];
		Record->mobilephone=InputField[6];
		Record->familyphone=InputField[7];
		Record->officephone=InputField[8];
		Record->bpnum=InputField[9];
		Record->faxnum=InputField[10];
		Record->category=InputField[11];
		Record->sound   = InputField[12];
		Record->qq		= InputField[13];
		
	}

	WORD *GetInputField(int nField)
	{
		return InputField[nField];
	}
	UINT GetInputFieldLen(int nField)
	{
		return InputFieldLen[nField];
	}

	BYTE GetOutputFieldId(int nField)
	{
		return OutputFieldId[nField];
	}

	void SetOutputField(int nField, CString csStr)
	{
		InputField[nField] = csStr;
 	}

}ContactsRecordWrapper;
*/

/////////////////////////////////////////////////////////////////////////////////////////////////
//Schedules
typedef struct _ScheduleDate
{
	int _yy;
	int _mm;
	int _dd;
	public:
	_ScheduleDate()
	{
		_yy = 0; _mm = 0; _dd = 0;
	}
	void SetDate(int yy, int mm, int dd)
	{
			_yy = yy; _mm = mm; _dd = dd;
	}

	void SetDate(_ScheduleDate *date)
	{
			_yy = date->_yy; _mm = _mm = date->_mm; _dd = date->_dd;
	}
	_ScheduleDate GetDate()
	{
		return *this;
	}
}ScheduleDate;

typedef struct _SchedulesRecord
{
	CString title; //Title of the shcedule
	CString starttime; //start time of schedule
	CString endtime; //end time of schedule
	CString memo; //notes of the schedule
	CString telnumber; //telephone number to be reminded on
	BOOL haveAlarm; //true or false
//	CString alarmLeadtime; //Similar to Reminder option of outlook. must be in minutes.
	BOOL haveRepeat; //true or false
	BOOL haveUntil; // true or false if there is an until date
	INT repeatType; //Daily , weekly, Monthly, MonthlyWeekly and Yearly
	UINT repeatFrequency; // every 1 day/week/month/year
	UINT rfWhichWeek; //First , Second, Third, Fourth or Last week
	UINT rfWhichDay; //Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday
	ScheduleDate untilDate;
	int nDayofWeekMask;
//Initialization
	CString reasonForReply;

	//Update for CAMMagic Mobile 2.0
	CString cuLocation; //Location of the shcedule
//	UINT uiIndex;//Index of the shcedule assiguned by mobile
	char szuiIndex[100];
	UINT uiNodeType;//NodeType of the shcedule
	UINT uiPrivate;//int uiPrivate
	BOOL bhaveTone;
//	CString csHaveAlarm;
//	CString csHaveRepeat;

	COleDateTime dtAlarmTime;
	BOOL bEndDateSameToStartDate; //ธ๓คั

	GUID pcid;
	CString csPcid;
	void SetPCID(GUID id)
	{
		pcid = id;
		csPcid = CString(GuidToString(id));
	}
	GUID GetPCID(void)
	{
		return pcid;
	}
	void SetRecord(CString _title, 
				CString _st,
				CString _et,
				CString _memo,
				CString _telephone,
				BOOL _haveAlarm,
				BOOL _haveRepeat,
				BOOL _haveUntil,
				UINT _repeatType,
				UINT _repeatFreq,
				UINT _whichweek,
				UINT _whichday,
				int _nDayofWeekMask,
				ScheduleDate   _untilDate,
				CString _reasonForReply,
				COleDateTime _dtAlarmTime,
				BOOL	_bEndDateSameToStartDate,
				char*           _uiIndex = NULL,
				CString _cuLocation = _T(""),
				UINT           _uiNodeType = 0,
				UINT           _uiPrivate = 0,
				BOOL           _bhaveTone = 0
//				CString _csHaveRepeat = _T(""),
//				CString _csHaveAlarm = _T("")
				)
	{
		title			= _title;
		starttime		= _st;
		endtime			= _et;
		memo			= _memo;
		telnumber		= _telephone;
		haveAlarm		= _haveAlarm;
		haveRepeat		= _haveRepeat;
		haveUntil		= _haveUntil;
		repeatType		= _repeatType; 
		repeatFrequency = _repeatFreq; 
		rfWhichWeek		= _whichweek; 
		rfWhichDay		= _whichday; 
		nDayofWeekMask = _nDayofWeekMask;
		untilDate.SetDate(&_untilDate);

		haveRepeat = _haveRepeat;
		haveAlarm = _haveAlarm;
//		csHaveRepeat = csHaveRepeat;
//		csHaveAlarm = csHaveRepeat;

		reasonForReply = _reasonForReply;

		//
		cuLocation = _cuLocation;
	//	uiIndex = _uiIndex;
		if(_uiIndex)
			sprintf(szuiIndex,_uiIndex);
		else
			itoa(0,szuiIndex,10);

        uiNodeType = _uiNodeType;
		uiPrivate = _uiPrivate;
		bhaveTone = _bhaveTone;
        dtAlarmTime = _dtAlarmTime;
		bEndDateSameToStartDate =_bEndDateSameToStartDate;

	}

	void SetRecord(_SchedulesRecord *sr)
	{
		title			= sr->title;
		starttime		= sr->starttime;
		endtime			= sr->endtime;
		memo			= sr->memo;
		telnumber		= sr->telnumber;
		haveAlarm		= sr->haveAlarm;
		haveRepeat		= sr->haveRepeat;
		haveUntil		= sr->haveUntil;
		repeatType		= sr->repeatType; 
		repeatFrequency = sr->repeatFrequency; 
		rfWhichWeek		= sr->rfWhichWeek; 
		rfWhichDay		= sr->rfWhichDay; 
		nDayofWeekMask  = sr->nDayofWeekMask;
		untilDate.SetDate(&sr->untilDate);

		haveAlarm = sr->haveAlarm;
		haveRepeat = sr->haveRepeat;
//		csHaveRepeat.Format("%d",sr->haveRepeat);
//		csHaveAlarm.Format("%d",sr->haveAlarm);
//		csHaveRepeat = sr->csHaveRepeat;
//		csHaveAlarm = sr->csHaveRepeat;

		reasonForReply = sr->reasonForReply;

		//
		cuLocation = sr->cuLocation;
	//	uiIndex = sr->uiIndex;
		sprintf(szuiIndex,sr->szuiIndex);
        uiNodeType = sr->uiNodeType;	
		uiPrivate =  sr->uiPrivate;
		bhaveTone = sr->bhaveTone;
        dtAlarmTime = sr->dtAlarmTime;
		bEndDateSameToStartDate = sr->bEndDateSameToStartDate;
	}


	_SchedulesRecord()
	{
		title = _T("");
		starttime = _T("");
		endtime = _T("");
		memo = _T("");
		telnumber = _T("");
		haveAlarm = FALSE;
		haveRepeat = FALSE;
		haveUntil = FALSE;
		repeatType	=0; 
		repeatFrequency = 0;
		rfWhichWeek = 0;
		rfWhichDay = 0;
		nDayofWeekMask = 0;
//		csHaveRepeat = _T("");
//		csHaveAlarm = _T("");
		pcid = IID_NONE;
		csPcid = _T("");
		reasonForReply = _T("");

		//
		cuLocation =  _T("");
	//	uiIndex = 0;
		itoa(0,szuiIndex,10);
        uiNodeType = 0;	//enum type
		uiPrivate =  0;
		bhaveTone = false;
		dtAlarmTime.m_dt = 0;
		bEndDateSameToStartDate = true;
	}
	void Reset()
	{ 
		title = _T("");
		starttime = _T("");
		endtime = _T("");
		memo = _T("");
		telnumber = _T("");
		haveAlarm = FALSE;
		haveRepeat = FALSE;
		haveUntil = FALSE;
		repeatType	=0; 
		repeatFrequency = 0;
		rfWhichWeek = 0;
		rfWhichDay = 0;
		nDayofWeekMask = 0;
//		csHaveRepeat = _T("");
//		csHaveAlarm = _T("");
		pcid = IID_NONE;
		csPcid = _T("");
		reasonForReply = _T("");

		//
		cuLocation =  _T("");
	//	uiIndex = 0;
		itoa(0,szuiIndex,10);
        uiNodeType = 0;	//enum type
		uiPrivate =  0;
		bhaveTone = false;
		dtAlarmTime.m_dt = 0;
		bEndDateSameToStartDate = true;
	}


	~_SchedulesRecord()
	{
		title = _T("");
		starttime = _T("");
		endtime = _T("");
		memo = _T("");
		telnumber = _T("");
		haveAlarm = FALSE;
		haveRepeat = FALSE;
		haveUntil = FALSE;
		repeatType	=0; 
		repeatFrequency = 0;
		rfWhichWeek = 0;
		rfWhichDay = 0;
		nDayofWeekMask =0 ;
		pcid = IID_NONE;
		csPcid = _T("");
		reasonForReply = _T("");
		
		//
		cuLocation =  _T("");
	//	uiIndex = 0;
		itoa(0,szuiIndex,10);
        uiNodeType = 0;	//enum type
		dtAlarmTime.m_dt = 0;
		bEndDateSameToStartDate = true;
	}

	_SchedulesRecord *GetRecord()
	{
		return this;
	}

}SchedulesRecord;
/*
typedef struct _ScheduleRecordWrapper
{
	WORD * InputField[SupportFieldNumSch];
	UINT InputFieldLen[SupportFieldNumSch];
	BYTE OutputFieldId[SupportFieldNumSch];
// Initialization
	_ScheduleRecordWrapper()
	{
	}

	void PrepareRecord(SchedulesRecord &Record)
	{
		if (Record.pcid==IID_NONE) {
			InputField[0] = NULL;
		} else { 
			InputField[0] = Record.csPcid;
		}
		InputField[1] = Record.starttime;
		InputField[2] = Record.endtime;
		InputField[3] = Record.title;
		InputField[4] = Record.memo;
		InputField[5] = Record.telnumber;
		InputField[6] = Record.csHaveAlarm;
		InputField[7] = Record.csHaveRepeat;
		InputField[8] = Record.alarmLeadtime;
		InputField[9] = Record.reasonForReply;

		//Uncomment the lines below when the Organizer API supports these
	/*	InputField[8] = Record.repeatType;
		InputField[9] = Record.repeatFrequency;
		InputField[10]= Record.rfWhichWeek;
		InputField[11]= Record.rfWhichDay; */
/*	}
	void Init()
	{
		InputFieldLen[0] = PCID_MAX_LEN;
		InputFieldLen[1] = SCH_TIMESTART_MAX_LEN;
		InputFieldLen[2] = SCH_TIMEEND_MAX_LEN;
		InputFieldLen[3] = SCH_TITLE_MAX_LEN;
		InputFieldLen[4] = SCH_MEMO_MAX_LEN;
		InputFieldLen[5] = SCH_TEL_MAX_LEN;
		InputFieldLen[6] = SCH_HAVEALARM_MAX_LEN;
		InputFieldLen[7] = SCH_HAVEREPEAT_MAX_LEN;
		InputFieldLen[8] = SCH_TIME_MAX_LEN;
		InputFieldLen[9] = SCH_MAX_LEN;
	

		//Uncomment the lines below when the Organizer API supports these
/*		InputFieldLen[8] = SCH_TEL_MAX_LEN;
		InputFieldLen[9] = SCH_HAVEALARM_MAX_LEN;
		InputFieldLen[10] = SCH_TIME_MAX_LEN;
		InputFieldLen[11] = SCH_HAVEREPEAT_MAX_LEN;
*/
/*		OutputFieldId[0] = MO_SCH_PCID; 
		OutputFieldId[1] = MO_SCH_TIMESTART; 
		OutputFieldId[2] = MO_SCH_TIMEEND; 
		OutputFieldId[3] = MO_SCH_STRMOTIVE; 
		OutputFieldId[4] = MO_SCH_STRNOTE; 
		OutputFieldId[5] = MO_SCH_STRTELNUM; 
		OutputFieldId[6] = MO_SCH_HAVEALARM; 
		OutputFieldId[7] = MO_SCH_HAVREPEAT; 
		OutputFieldId[8] = MO_SCH_ALARMLEADTIME; 
		OutputFieldId[9] = MO_SCH_REASONFORREPLY; 
*/

		//Uncomment the lines below when the Organizer API supports these
/*		OutputFieldId[0] = 9; 
		OutputFieldId[1] = 10; 
		OutputFieldId[2] = 11; 
		OutputFieldId[3] = 12; 
*/
/*	}
	void GetRecord(SchedulesRecord *Record)
	{
		Record->csPcid = InputField[0];
		Record->starttime=InputField[1];
		Record->endtime=InputField[2];
		Record->title = InputField[3];
		Record->memo=InputField[4] ;
		Record->telnumber=InputField[5];
		Record->haveAlarm=atoi(CString(InputField[6]));
		Record->alarmLeadtime = InputField[7];
		Record->haveRepeat=atoi(CString(InputField[8])) ;
		
		Record->reasonForReply=InputField[9];
		//Uncomment the lines below when the Organizer API supports these
/*		Record->starttime=InputField[8];
		Record->endtime=InputField[9];
		Record->title = InputField[10];
		Record->memo=InputField[11] ;
*/
/*	}

	WORD *GetInputField(int nField)
	{
		return InputField[nField];
	}
	UINT GetInputFieldLen(int nField)
	{
		return InputFieldLen[nField];
	}

	BYTE GetOutputFieldId(int nField)
	{
		return OutputFieldId[nField];
	}

	void SetOutputField(int nField, CString csStr)
	{
	//	InputField[nField] = new WORD[csStr.GetLength()+1];
		InputField[nField] = csStr;
 	//	AfxMessageBox(CString(InputField[nField]));

	}

}ScheduleRecordWrapper;
*/
/////////////////////////////////////////////////////////////////////////////////////////////////
//Notes

typedef struct _NotesRecord
{
	CString title;
	CString memo;
//Initialization
	GUID pcid;
	CString csPcid;
	void SetPCID(GUID id)
	{
		pcid = id;
		csPcid = CString(GuidToString(id));
	}
	GUID GetPCID(void)
	{
		return pcid;
	}
	void SetRecord(CString _title, 
				CString _memo
				)
	{
		title = _title;
		memo = _memo;
	}


	_NotesRecord()
	{
		title = _T("");
		memo = _T("");
		pcid = IID_NONE;
		csPcid = _T("");
	}

	~_NotesRecord()
	{
		title = _T("");
		memo = _T("");
		pcid = IID_NONE;
		csPcid = _T("");
	}

	_NotesRecord *GetRecord()
	{
		return this;
	}

}NotesRecord;
/*
typedef struct _NotesRecordWrapper
{
	WORD * InputField[SupportFieldNumNotes];
	UINT InputFieldLen[SupportFieldNumNotes];
	BYTE OutputFieldId[SupportFieldNumNotes];
// Initialization
	_NotesRecordWrapper()
	{
	}

	void PrepareRecord(NotesRecord &Record)
	{

		if (Record.pcid==IID_NONE) {
			InputField[0] = NULL;
		} else { 
			InputField[0] = Record.csPcid;
		}
		InputField[1] = Record.title;
		InputField[2] = Record.memo;
	}
	void Init()
	{
		InputFieldLen[0] = PCID_MAX_LEN;
		InputFieldLen[1] = TITLE_MAX_LEN;
/*		InputFieldLen[2] = MEMO_MAX_LEN;

		OutputFieldId[0] = MO_NOTE_PCID; 
		OutputFieldId[1] = MO_NOTE_TITLE; 
		OutputFieldId[2] = MO_NOTE_MEMO; 
*//*	}
	void GetRecord(NotesRecord *Record)
	{

		Record->csPcid = InputField[0];

		Record->title = InputField[1];
		Record->memo=InputField[2] ;
			

	}
/*
	WORD **GetInputFields()
	{
		return InputField;
	}
	
	UINT []GetInputFieldsLen()
	{
		return InputFieldLen;
	}

	BYTE GetOutputFieldsId()
	{
		return OutputFieldId;
	}
*/
/*	WORD *GetInputField(int nField)
	{
		return InputField[nField];
	}
	UINT GetInputFieldLen(int nField)
	{
		return InputFieldLen[nField];
	}

	BYTE GetOutputFieldId(int nField)
	{
		return OutputFieldId[nField];
	}

	void SetOutputField(int nField, CString csStr)
	{
//		InputField[nField] = new WORD[csStr.GetLength()+1];
		InputField[nField] = csStr;
 	//	AfxMessageBox(CString(InputField[nField]));

	}

}NotesRecordWrapper;
*/

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function prototypes for sync
/*
typedef long (*_MoOpenMyOrganizer)(DWORD dwLogEnable, LPCTSTR pszProductPath);
typedef long (*_MoCloseMyOrganizer)(void);
typedef long (*_MoOpenFile)(LPCTSTR lpszFileName);
typedef long (*_MoCloseFile)(void);
typedef long (*_MoOpenDatabase)(DWORD dwID);
typedef long (*_MoCloseDatabase)(void);
typedef long (*_MoCreateRecord)(DWORD dwDesiredAccess, LPTSTR lpIDBuffer,DWORD dwIDBufferLength,DWORD &dwWriteBytes);
typedef long (*_MoGetRecordFieldNameByID)(DWORD dwDatabaseID, DWORD dwFieldID, LPTSTR lpInBuffer,DWORD dwInBufferLength,DWORD &dwWriteBytes);
typedef long (*_MoSetRecordField)(LPTSTR lpFieldName,LPTSTR lpFieldContent);
typedef long (*_MoGetRecordField)(LPTSTR lpFieldName,LPTSTR lpFieldBuffer, DWORD dwFieldBufferLength, DWORD &dwWriteBytes);
typedef long (*_MoCloseRecord)(BOOL isSave,LPTSTR lpInBuffer);
typedef long (*_MoGetRecordCount)(int &count);
typedef long (*_MoSetGeneralRecordRecurInfo)(SYTR_REPEAT structRepeat);
typedef long (*_MoGetGeneralRecordRecurInfo)(SYTR_REPEAT &structRepeat, BOOL &bRepeat);
*/
#endif //AFX_CCAMMSYNCINCLUDE_H__F3DF53D9_6EAF_44BF_A2B0_AD9452E695BD__INCLUDED_
