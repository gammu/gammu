#if !defined(AFX_CAMMSCHEDULEEDITORDLG_H__E7FB4854_7FE1_4161_84B1_A1E477E2C23A__INCLUDED_)
#define AFX_CAMMSCHEDULEEDITORDLG_H__E7FB4854_7FE1_4161_84B1_A1E477E2C23A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "..\OrganizerModules.h"
#include "..\resource.h"
#include "..\Organizer\OrganizeDeclaration.h"
#include "..\Organizer\ScheduleInfo.h"
#include "..\Organizer\ScheduleTimeInfo.h"
#include "..\Organizer\ScheduleMonthInfo.h"
#include "..\myedit.h"
#include "..\MobileDevice\CAMMSync.h"

// CAMMScheduleEditorDlg.h : header file
//
typedef enum {TYPE_SEC=0, TYPE_MIN=1, TYPE_HOUR, TYPE_DAY, TYPE_WEEK, TYPE_MONTH, TYPE_YEAR} TimeLengthType;
/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleEditorDlg dialog
#define WM_CHECKVIEWMODE WM_USER+111 
class CCAMMScheduleEditorDlg : public CDialog
{
// Construction
private:
	HBRUSH m_hBrush;
	HFONT	  m_hFont;
	BOOL m_bViewOnly;
public:
	CCAMMScheduleEditorDlg(CWnd* pParent = NULL);   // standard constructor
	~CCAMMScheduleEditorDlg();
	CString m_ExtraBytes;

	CString	m_DialogTitle;
	void SetDialogTitle(CString _DialogTitle) {m_DialogTitle = _DialogTitle;}

	bool m_bShowCtrlItem;
	void SetShowCtrlItem(bool _bShowCtrlItem) {m_bShowCtrlItem = _bShowCtrlItem;}
    void ShowCtrlItem(bool _bShowCtrlItem); 
    void Check_NoteType(); 
    bool GetCalendarSupport();
    void InsertNoteTypetoComboCtrl();
	void InsertRepeatTypetoComboCtrl(DWORD dwFalg);
	void InsertAlarmLeadTypetoComboCtrl(DWORD dwFalg);
	void InsertEndTimeIntervalTypetoComboCtrl(DWORD dwFalg);
	void InsertAlarmTimeIntervalTypetoComboCtrl(DWORD dwFalg);
	BOOL IsEnabledAlarmTime(); 


// Dialog Data
	//{{AFX_DATA(CCAMMScheduleEditorDlg)
	enum { IDD = IDD_ORG_SCHEDULE_EDITOR };
	CSpinButtonCtrl	m_spRepeatinterval;
	CEdit	m_edRepeatInterval;
	CStaticEx	m_scRepeatON;
	CStaticEx	m_scRepeatinterval;
	//ÐÇÆÚ¸´Ñ¡°´Å¥£¬ use in 99u kerm 12.27
	CCheckEx2	m_chWeek7;		
	CCheckEx2	m_chWeek6;
	CCheckEx2	m_chWeek5;
	CCheckEx2	m_chWeek4;
	CCheckEx2	m_chWeek3;
	CCheckEx2	m_chWeek2;
	CCheckEx2	m_chWeek1;
	CEdit	m_edDescription;
	CStaticEx	m_stDescription;
	CCheckEx2	m_chkRepeatEndDate;
	CStaticEx	m_stRepeatEndData;
	CComboBox	m_cbAlarmTimeType;
	CEdit	m_edtAlarmTime;
	CComboBox	m_cbStopTimeType;
	CEdit	m_edtStopTime;
	CComboBox m_cbAlarmTime;
	CComboBox m_cbAlarmday;
	CComboBox m_cbStartDate;
	CStaticEx	  m_stLtType;
	CStaticEx	  m_stLtTitle;
	CStaticEx	  m_stAlarmPhoneNo;
	CStaticEx     m_stStartTime;
	CStaticEx     m_stStopTime;
	CStaticEx     m_stPriority;		//add priority static txt
	CCheckEx2	  m_chkAlarm;
	CCheckEx2	  m_chkRepeat;
	CRescaleButton m_ctrlOk;
	CRescaleButton m_ctrlCancel;
	CCheckEx		m_ckTone;
	CComboBox		m_cbNoteType;
	CStatic   m_stLocation;
	CComboBox m_cbStopDate;
	CEdit     m_etLocation;
	CMyEdit	m_etAlarmPhoneNo;
	CComboBox	m_cbAlarmLeadTime;
	CComboBox	m_cbRepeatWhichWeek;
	CComboBox	m_cbWeekDay;
	CComboBox	m_cbRepeatType;
	CComboBox	m_CbStopTime;
	CComboBox	m_CbStartTime;
	CComboBox   m_cbPriority;		//add priority combo 
	int     m_cbIndex_NoteType;
//	CString	m_Memo;
	CString	m_Title;
	BOOL	m_bAlarm;
	BOOL	m_bRepeat;
	BOOL	m_bRepeatUntil;
	CString	m_AlarmPhoneNo;
	int		m_RepeatType;
	int		m_WhichDay;
	int		m_RepeatWhichWeek;
	COleDateTime	m_UntilDate;
	COleDateTime	m_StartDate;
	CString	        m_csLocation;
	COleDateTime	m_StopDate;
	BOOL            m_bTone;
	COleDateTime	m_Alarmday;
	COleDateTime    m_AlarmTime;
	int		m_AlarmLength;
	int		m_nStopLength;
	//}}AFX_DATA
	TCHAR m_szProfile[MAX_PATH];

	DWORD GetTimeInterval(CString strStartStr,CString strEndStr );

	int m_nMaxAlarmLength;
	int m_nMaxStopLength;
	int m_nDayofWeekMask;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCAMMScheduleEditorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	void DDX_CUnicodeString(CDataExchange* pDX,int nID,CString &uStr);
	void DDV_CUnicodeStringMinMax(CDataExchange* pDX,int nID,CString &uStr,int nMin,int nMax);
    void DDX_CCheckEx2(CDataExchange* pDX,int nID,BOOL &bEnable);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCAMMScheduleEditorDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeSchDataStarttime();
	afx_msg void OnSelchangeSchDataStoptime();
	afx_msg void OnSchChkAlarm();
	afx_msg void OnSchChkRepeat();
	afx_msg void OnSCHCHKREPEATUntil();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelchangeSchDataRepeatType();
	virtual void OnCancel();
	afx_msg void OnUpdateSchDataAlarmPhoneno();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelchangeNotetype();
	afx_msg void OnSelchangeAlarmInterval();
	afx_msg void OnDatetimechangeSchDataStartdate(NMHDR* pNMHDR, LRESULT* pResult);
	//kerm add for 9a9u 1.16
	afx_msg void OnDatetimechangeSchDataEnddate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusSchDataStarttime();
	afx_msg void OnKillfocusSchDataStoptime();
	afx_msg void OnKillfocusSchDataAlarmtimeedit();
	afx_msg void OnKillfocusSchDataStoptimeedit();
	afx_msg void OnChangeEdRepeateinter();
	afx_msg void OnKillfocusEdRepeateinter();
	afx_msg void OnSelchangeSchDataAlarmtimetype();
	//}}AFX_MSG
	afx_msg LRESULT OnCheckViewMode(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

public:
	void DisableAllControl();
	void SetViewOnlyMode(bool bViewOnly);
	BOOL GetIntervalTypenValue_Day(COleDateTime value1,COleDateTime value2 , TimeLengthType& nType,int& dwIntervalLength,
		DWORD MaxDayValue,DWORD MaxWeekValue);

	void UpdateDateofWeekCtlStates();
	void SetRepeatType(int nType,int nDayofWeekMask);
//	BOOL UpdateStopTime(DWORD dwTimeIntervalMin,int nStartTimeIndex);
	BOOL GetIntervalTypenValue(COleDateTime value1,COleDateTime value2 , TimeLengthType& nType,int& dwIntervalLength,
		DWORD MaxDayValue,DWORD MaxWeekValue);
	BOOL SetStopIntervalValue(COleDateTime dtStartDateTime,COleDateTime dtStopDateTime);
	BOOL SetAlarmIntervalValue(COleDateTime dtStartDateTime,COleDateTime dtAlarmDateTime);
	BOOL GetStopDateTime(COleDateTime & dtStopTime);
	BOOL  GetAlarmDateTime(COleDateTime dtStart,COleDateTime& dtAlarm,BOOL& bTimeOnly);
	BOOL IsStopDateTimelegal();
	int m_nInitAlarmLeadTime;
	DWORD GetAlarmLeadTime();
	DWORD GetStartEndTimeInterval();
	void SetMonthInfoPtr(CScheduleMonthInfo* pMi);
	void SetScheduleInfo(CScheduleInfo sd);
	void GetScheduleInfo(CScheduleInfo& sd);

private:
	void xInitDialog();
	BOOL xCheckStringLegal(CString s);
	BOOL xCheckPhoneLegal(CString s);
	BOOL xIsDataModified();
	BOOL xGenerateSchedule();
	ScheduleDate xGetScheduleDate(COleDateTimeEx utilWhichDate);
	CScheduleMonthInfo* m_pMonthInfo;
	void xUpdateStopTimeCb(int startIndex);
	int xGetTimeIndex(CScheduleTimeInfo hi);
	CScheduleTimeInfo xGetHourInfoByTimeIndex(int timeIndex);
	void xUpdateData(BOOLEAN bControl2Variable);
	void xGetDlgItemText(INT ControlId, CString& ExchangeString, INT MaxLength);

	CScheduleInfo m_ScheduleInfo;
	BOOL m_bCreateNew;
    CArray<NoteTypeSupportInfo,NoteTypeSupportInfo> m_ArrayofSupportNoteType;
	int m_SizeofSupportNoteType;
	DWORD m_dwAlarmLeadTypeFlag;
	DWORD m_dwRepeatTypeFlag;

	DWORD m_dwStopTimeIntervalTypeFlag;
	DWORD m_dwAlarmTimeIntervalTypeFlag;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMMSCHEDULEEDITORDLG_H__E7FB4854_7FE1_4161_84B1_A1E477E2C23A__INCLUDED_)
