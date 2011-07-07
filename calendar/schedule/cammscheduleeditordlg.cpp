// CAMMScheduleEditorDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "..\camm.h"
#include "CAMMScheduleEditorDlg.h"
#include "..\Calendar.h"
#include "..\loadstring.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleEditorDlg dialog


CCAMMScheduleEditorDlg::CCAMMScheduleEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCAMMScheduleEditorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCAMMScheduleEditorDlg)
	m_Title = _T("");
	m_bAlarm = FALSE;
	m_bRepeat = FALSE;
	m_AlarmPhoneNo = _T("");
	m_WhichDay = -1;
	m_RepeatWhichWeek = -1;
	m_UntilDate = COleDateTime::GetCurrentTime();
	m_StartDate = COleDateTime::GetCurrentTime();

	m_StopDate  = COleDateTime::GetCurrentTime();//Calendar
	m_Alarmday  = COleDateTime::GetCurrentTime();//Calendar
	m_AlarmTime = COleDateTime::GetCurrentTime();//Calendar
	m_AlarmLength = 0;
	m_nStopLength = 0;
	//}}AFX_DATA_INIT
	m_bCreateNew = TRUE;
	m_ExtraBytes = _T("");
	m_DialogTitle = _T("");

	m_bShowCtrlItem = TRUE;
	m_hBrush = NULL;
	//Calendar
	m_bTone = false;
	m_cbIndex_NoteType = GSM_CAL_REMINDER-1;

	m_dwAlarmLeadTypeFlag = 0;
	m_dwRepeatTypeFlag= 0;
	m_nMaxAlarmLength = 1;
	m_nMaxStopLength= 1;
	m_dwStopTimeIntervalTypeFlag = 0;
	m_dwAlarmTimeIntervalTypeFlag = 0;
	m_nDayofWeekMask = 0;

	m_hFont = NULL;
	m_bViewOnly = false;


}
CCAMMScheduleEditorDlg::~CCAMMScheduleEditorDlg()
{
	if(m_hBrush)
		DeleteObject(m_hBrush);
	if(m_hFont)
		::DeleteObject(m_hFont);
}
void CCAMMScheduleEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCAMMScheduleEditorDlg)
	DDX_Control(pDX, IDC_SP_REPEATINTER, m_spRepeatinterval);
	DDX_Control(pDX, IDC_ED_REPEATEINTER, m_edRepeatInterval);
	DDX_Control(pDX, IDC_SC_REPEATON, m_scRepeatON);
	DDX_Control(pDX, IDC_SC_REPEATE_INTERVAL, m_scRepeatinterval);
	DDX_Control(pDX, IDC_CHWEEK7, m_chWeek7);
	DDX_Control(pDX, IDC_CHWEEK6, m_chWeek6);
	DDX_Control(pDX, IDC_CHWEEK5, m_chWeek5);
	DDX_Control(pDX, IDC_CHWEEK4, m_chWeek4);
	DDX_Control(pDX, IDC_CHWEEK3, m_chWeek3);
	DDX_Control(pDX, IDC_CHWEEK2, m_chWeek2);
	DDX_Control(pDX, IDC_CHWEEK1, m_chWeek1);
	DDX_Control(pDX, IDC_ED_DESCRIPTION, m_edDescription);
	DDX_Control(pDX, IDC_SC_DESCRIPTION, m_stDescription);
	DDX_Control(pDX, IDC_SCH_CHK_REPEAT_Until, m_chkRepeatEndDate);
	DDX_Control(pDX, IDC_REPEATENDSTATIC, m_stRepeatEndData);
	DDX_Control(pDX, IDC_SCH_DATA_ALARMTIMETYPE, m_cbAlarmTimeType);
	DDX_Control(pDX, IDC_SCH_DATA_ALARMTIMEEDIT, m_edtAlarmTime);
	DDX_Control(pDX, IDC_SCH_DATA_STOPTIMETYPE, m_cbStopTimeType);
	DDX_Control(pDX, IDC_SCH_DATA_STOPTIMEEDIT, m_edtStopTime);
	DDX_Control(pDX, IDC_SCH_DATA_ALARMTIME, m_cbAlarmTime);
	DDX_Control(pDX, IDC_SCH_DATA_ALARMDAY, m_cbAlarmday);
	DDX_Control(pDX, IDC_SCH_DATA_STARTDATE, m_cbStartDate);
	DDX_Control(pDX, IDC_SCH_LT_TYPE, m_stLtType);
	DDX_Control(pDX, IDC_SCH_LT_TITLE, m_stLtTitle);
	DDX_Control(pDX, IDC_SCH_LT_REMIND_PHONENO, m_stAlarmPhoneNo);
	DDX_Control(pDX, IDC_SCH_LT_STARTTIME, m_stStartTime);
	DDX_Control(pDX, IDC_SCH_LT_STOPTIME, m_stStopTime);
	DDX_Control(pDX, IDC_STC_CAL_PRIORITY, m_stPriority);	//add priority
	DDX_Control(pDX, IDC_SCH_CHK_ALARM, m_chkAlarm);
	DDX_Control(pDX, IDC_SCH_CHK_REPEAT, m_chkRepeat);
	DDX_Control(pDX, IDOK, m_ctrlOk);
	DDX_Control(pDX, IDCANCEL, m_ctrlCancel);
	DDX_Control(pDX, IDC_SCH_CHK_TONE, m_ckTone);
	DDX_Control(pDX, IDC_SCH_DATA_NoteType, m_cbNoteType);
	DDX_Control(pDX, IDC_CMB_CAL_PRIORITY, m_cbPriority);	//add priority combo
	DDX_Control(pDX, IDC_SCH_LT_LOCATION, m_stLocation);
	DDX_Control(pDX, IDC_SCH_DATA_STOPDATE, m_cbStopDate);
	DDX_Control(pDX, IDC_SCH_DATA_LOCATION, m_etLocation);
	DDX_Control(pDX, IDC_SCH_DATA_ALARM_PHONENO, m_etAlarmPhoneNo);
	DDX_Control(pDX, IDC_SCH_DATA_ALARM_INTERVAL, m_cbAlarmLeadTime);
	DDX_Control(pDX, IDC_SCH_DATA_REPEAT_WHICHWEEK, m_cbRepeatWhichWeek);
	DDX_Control(pDX, IDC_SCH_DATA_REPEAT_WEEKDAY, m_cbWeekDay);
	DDX_Control(pDX, IDC_SCH_DATA_REPEAT_TYPE, m_cbRepeatType);
	DDX_Control(pDX, IDC_SCH_DATA_STOPTIME, m_CbStopTime);
	DDX_Control(pDX, IDC_SCH_DATA_STARTTIME, m_CbStartTime);
	DDX_CBIndex(pDX, IDC_SCH_DATA_NoteType, m_cbIndex_NoteType);
	DDX_CCheckEx2(pDX, IDC_SCH_CHK_ALARM, m_bAlarm);
	DDX_CCheckEx2(pDX, IDC_SCH_CHK_REPEAT, m_bRepeat);
	DDX_CBIndex(pDX, IDC_SCH_DATA_REPEAT_WEEKDAY, m_WhichDay);
	DDX_CBIndex(pDX, IDC_SCH_DATA_REPEAT_WHICHWEEK, m_RepeatWhichWeek);
	DDX_DateTimeCtrl(pDX, IDC_SCH_DATA_REPEAT_WHICHDATE, m_UntilDate);
	DDX_DateTimeCtrl(pDX, IDC_SCH_DATA_STARTDATE, m_StartDate);
	DDX_DateTimeCtrl(pDX, IDC_SCH_DATA_STOPDATE, m_StopDate);
	DDX_CCheckEx2(pDX, IDC_SCH_CHK_TONE, m_bTone);
	DDX_DateTimeCtrl(pDX, IDC_SCH_DATA_ALARMDAY, m_Alarmday);
	DDX_DateTimeCtrl(pDX, IDC_SCH_DATA_ALARMTIME, m_AlarmTime);
	DDX_Text(pDX, IDC_SCH_DATA_ALARMTIMEEDIT, m_AlarmLength);
	DDX_Text(pDX, IDC_SCH_DATA_STOPTIMEEDIT, m_nStopLength);
	DDV_MinMaxInt(pDX, m_nStopLength, 0, m_nMaxStopLength);
	//}}AFX_DATA_MAP
	DDX_CUnicodeString(pDX,IDC_SCH_DATA_TITLE,m_Title);
//	DDV_CUnicodeStringMinMax(pDX,IDC_SCH_DATA_TITLE,m_Title,0,320);
	DDX_CUnicodeString(pDX,IDC_SCH_DATA_ALARM_PHONENO,m_AlarmPhoneNo);
//	DDX_CUnicodeString(pDX,IDC_SCH_DATA_REPEAT_INTERVAL,m_RepeatFrequency);
	//calendar
	DDX_Text(pDX, IDC_SCH_DATA_LOCATION, m_csLocation);
}
void CCAMMScheduleEditorDlg::DDX_CUnicodeString(CDataExchange* pDX,int nID,CString &uStr)
{
	if (pDX->m_bSaveAndValidate) {
		TCHAR lStr[MAX_LENGTH];
		memset(lStr,0x00,sizeof(TCHAR)*MAX_LENGTH);
		GetDlgItemText(nID,lStr,MAX_LENGTH);
		uStr = lStr;
	} else {
		SetDlgItemText(nID,uStr);
	}
}
void CCAMMScheduleEditorDlg::DDV_CUnicodeStringMinMax(CDataExchange* pDX,int nID,CString &uStr,int nMin,int nMax)
{
	if (!pDX->m_bSaveAndValidate)
	{
		//	} else {//if (pDX->m_hWndLastControl != NULL && pDX->m_bEditLastControl) {
		// limit the control max-chars automatically
		CWnd* pCtrl = GetDlgItem(nID);	
		::SendMessage(pCtrl->GetSafeHwnd(), EM_LIMITTEXT, nMax, 0);
	}
}

void CCAMMScheduleEditorDlg::DDX_CCheckEx2(CDataExchange* pDX,int nID,BOOL &bEnable)
{
	if (pDX->m_bSaveAndValidate) {
    	CCheckEx2* pCtrl=NULL;
		pCtrl = (CCheckEx2*) GetDlgItem(nID);
		if (pCtrl)
    	    bEnable = pCtrl->GetCheck();
    	//bEnable = pDX->m_bSaveAndValidate;
	}
}


BEGIN_MESSAGE_MAP(CCAMMScheduleEditorDlg, CDialog)
	//{{AFX_MSG_MAP(CCAMMScheduleEditorDlg)
	ON_CBN_SELCHANGE(IDC_SCH_DATA_STARTTIME, OnSelchangeSchDataStarttime)
	ON_CBN_SELCHANGE(IDC_SCH_DATA_STOPTIME, OnSelchangeSchDataStoptime)
	ON_BN_CLICKED(IDC_SCH_CHK_ALARM, OnSchChkAlarm)
	ON_BN_CLICKED(IDC_SCH_CHK_REPEAT, OnSchChkRepeat)
	ON_BN_CLICKED(IDC_SCH_CHK_REPEAT_Until, OnSCHCHKREPEATUntil)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_SCH_DATA_REPEAT_TYPE, OnSelchangeSchDataRepeatType)
	ON_EN_UPDATE(IDC_SCH_DATA_ALARM_PHONENO, OnUpdateSchDataAlarmPhoneno)
	ON_WM_CTLCOLOR()
	ON_CBN_SELCHANGE(IDC_SCH_DATA_NoteType, OnSelchangeNotetype)
	ON_CBN_SELCHANGE(IDC_SCH_DATA_ALARM_INTERVAL, OnSelchangeAlarmInterval)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_SCH_DATA_STARTDATE, OnDatetimechangeSchDataStartdate)
	//增加对结束时间变化的响应 kerm add for 9a9u 1.16
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_SCH_DATA_STOPDATE, OnDatetimechangeSchDataEnddate)
	ON_CBN_KILLFOCUS(IDC_SCH_DATA_STARTTIME, OnKillfocusSchDataStarttime)
	ON_CBN_KILLFOCUS(IDC_SCH_DATA_STOPTIME, OnKillfocusSchDataStoptime)
	ON_EN_KILLFOCUS(IDC_SCH_DATA_ALARMTIMEEDIT, OnKillfocusSchDataAlarmtimeedit)
	ON_EN_KILLFOCUS(IDC_SCH_DATA_STOPTIMEEDIT, OnKillfocusSchDataStoptimeedit)
	ON_EN_CHANGE(IDC_ED_REPEATEINTER, OnChangeEdRepeateinter)
	ON_EN_KILLFOCUS(IDC_ED_REPEATEINTER, OnKillfocusEdRepeateinter)
	ON_CBN_SELCHANGE(IDC_SCH_DATA_ALARMTIMETYPE, OnSelchangeSchDataAlarmtimetype)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CHECKVIEWMODE, OnCheckViewMode)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleEditorDlg message handlers

BOOL CCAMMScheduleEditorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	int iSize = 0;
	TCHAR szName[MAX_PATH];
	CFont* pFont = NULL;
	if(GetProfileFont(NULL,NULL,iSize,szName))
	{
		iSize = -iSize ;
		if(m_hFont==NULL)
			m_hFont = GetFontEx(szName,iSize);
		pFont = CFont::FromHandle(m_hFont);
	}

	m_ExtraBytes = _T("");
//	m_hBrush=::CreateSolidBrush(RGB(235, 243, 246));

	m_cbAlarmLeadTime.ResetContent();
	m_cbRepeatWhichWeek.ResetContent();
	m_cbPriority.ResetContent();
	m_cbWeekDay.ResetContent();
	m_cbRepeatType.ResetContent();
	m_CbStopTime.ResetContent();
	m_CbStartTime.ResetContent();
	m_cbPriority.ResetContent();	//add priority combo
	int i=0;
	//初始化4个日期控件的显示格式
	CDateTimeCtrl * pDateCtl = (CDateTimeCtrl *)(GetDlgItem(IDC_SCH_DATA_STARTDATE));
	pDateCtl->SetFormat(_T("yyyy/MM/dd ddd"));
	if(pFont) pDateCtl->SetFont(pFont);
	pDateCtl = (CDateTimeCtrl *)(GetDlgItem(IDC_SCH_DATA_STOPDATE));
	pDateCtl->SetFormat(_T("yyyy/MM/dd ddd"));
	if(pFont) pDateCtl->SetFont(pFont);
	pDateCtl = (CDateTimeCtrl *)(GetDlgItem(IDC_SCH_DATA_ALARMDAY));
	pDateCtl->SetFormat(_T("yyyy/MM/dd ddd"));
	if(pFont) pDateCtl->SetFont(pFont);
	pDateCtl = (CDateTimeCtrl *)(GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE));
	pDateCtl->SetFormat(_T("yyyy/MM/dd ddd"));
	if(pFont) pDateCtl->SetFont(pFont);
	//for skin
	// 从ini读取控件的布局参数 只是调整布局没有对相应的空间赋值
	xInitDialog();
	//设置各控件使用的字体
	if(pFont)
	{
		m_edRepeatInterval.SetFont(pFont);
		m_edDescription.SetFont(pFont);
		m_cbAlarmTimeType.SetFont(pFont);
		m_edtAlarmTime.SetFont(pFont);
		m_cbStopTimeType.SetFont(pFont);
		m_edtStopTime.SetFont(pFont);
		m_cbAlarmTime.SetFont(pFont);
		m_cbAlarmday.SetFont(pFont);
		m_cbStartDate.SetFont(pFont);
		m_cbNoteType.SetFont(pFont);
		m_cbNoteType.SetFont(pFont);
		m_cbStopDate.SetFont(pFont);;
		m_etLocation.SetFont(pFont);;
		m_etAlarmPhoneNo.SetFont(pFont);;
		m_cbAlarmLeadTime.SetFont(pFont);;
		m_cbRepeatWhichWeek.SetFont(pFont);;
		m_cbPriority.SetFont(pFont);	//add priority for 9@9u
		m_cbWeekDay.SetFont(pFont);;
		m_cbRepeatType.SetFont(pFont);;
		m_CbStopTime.SetFont(pFont);;
		m_CbStartTime.SetFont(pFont);;
		m_cbPriority.SetFont(pFont);	//add priority
		GetDlgItem(IDC_SCH_DATA_TITLE)->SetFont(pFont);;
	}
	//sc ---sync   supportinfo
	/*
	typedef struct
	{
		int		SupportNodeNumber;
		int		TotalCalendar;
		int     Title_Boundry;
		int     Description_Boundry;
		int     Location_Boundry;
		DWORD   NodeFormat;
		int     CharType;       
	}CalendarSupportInfo;
	*/
	//下面代码没有用到，调用GetCalendarSupport(SupportInfo) 是没有效果的  暂时不用
	/*
	CalendarSupportInfo SupportInfo;
	CCAMMSync sc;
	sc.GetCalendarSupport(SupportInfo);

	int nCharType = 2;
	if(SupportInfo.CharType != 0 )
		nCharType = SupportInfo.CharType;
	//主题
	if(SupportInfo.Title_Boundry > 0)
	{((CEdit*) GetDlgItem(IDC_SCH_DATA_TITLE))->SetLimitText(nCharType * SupportInfo.Title_Boundry);}
	if(SupportInfo.Location_Boundry >0)
		m_etLocation.SetLimitText(nCharType*SupportInfo.Location_Boundry);
	if(SupportInfo.Description_Boundry >0)
		m_edDescription.SetLimitText(nCharType*SupportInfo.Description_Boundry);*/
/*	if(SupportInfo.Title_Boundry > 0 && SupportInfo.Location_Boundry >0)
	{
		((CEdit*) GetDlgItem(IDC_SCH_DATA_TITLE))->SetLimitText(2 * SupportInfo.Title_Boundry);
		m_etLocation.SetLimitText(2*SupportInfo.Location_Boundry);
	}*/

 	//Check all Note-Types supportted
    //typedef enum {REMIND=0, CALL, MEETING, BIRTHDAY, MEMO} NOTETYPE;//Calendar


	//调用GetCalendarSupport 完成对 初始化工作在这里完成
	//实际上调用了sync的GetCalendarSupport(m_SizeofSupportNoteType, m_ArrayofSupportNoteType))
    if (!GetCalendarSupport()) 
	{
//		AfxMessageBox("Failed to check all Note-Types supportted!");//return false;
		CDialog::OnCancel();
		return FALSE;
	}
	else 
	{
   		//Load string of NoteType from profile
		//读取lang\calendar.ini   
		//根据类型的id读取上边ini文件中的public段中的内容，插入到类型组合框中
		InsertNoteTypetoComboCtrl();
	}



/*	for (i=0;i<48;i++) {
		hourStr = LoadStringFromFile(ghSkinResource,IDS_ORG_SCHEDULE_HOUR_1200AM + i);
		m_CbStartTime.AddString(hourStr);
		m_CbStopTime.AddString(hourStr);
	}*/
	//初始化开始结束时间组合控件的显示内容
	CString strTime =_T("");
	CString hourStr;
	CString strAM = LoadStringFromFile(_T("public"),_T("AM"));
	CString strPM = LoadStringFromFile(_T("public"),_T("PM"));
	strTime = strAM;
	//下拉菜单的内容
	//让12点在前面，单独拿出来初始化
	hourStr.Format(_T("%s %02d:%02d"),strAM,12,0);
	m_CbStartTime.AddString(hourStr);
	m_CbStopTime.AddString(hourStr);

	hourStr.Format(_T("%s %02d:%02d"),strAM,12,30);
	m_CbStartTime.AddString(hourStr);
	m_CbStopTime.AddString(hourStr);
	//从1点到11点的初始化
	for (i=1;i<12;i++)
	{
		hourStr.Format(_T("%s %02d:%02d"),strAM,i,0);
		m_CbStartTime.AddString(hourStr);
		m_CbStopTime.AddString(hourStr);

		hourStr.Format(_T("%s %02d:%02d"),strAM,i,30);
		m_CbStartTime.AddString(hourStr);
		m_CbStopTime.AddString(hourStr);
	}
	//初始化 下午
	hourStr.Format(_T("%s %02d:%02d"),strPM,12,0);
	m_CbStartTime.AddString(hourStr);
	m_CbStopTime.AddString(hourStr);

	hourStr.Format(_T("%s %02d:%02d"),strPM,12,30);
	m_CbStartTime.AddString(hourStr);
	m_CbStopTime.AddString(hourStr);

	for (i=1;i<12;i++)
	{
		hourStr.Format(_T("%s %02d:%02d"),strPM,i,0);
		m_CbStartTime.AddString(hourStr);
		m_CbStopTime.AddString(hourStr);

		hourStr.Format(_T("%s %02d:%02d"),strPM,i,30);
		m_CbStartTime.AddString(hourStr);
		m_CbStopTime.AddString(hourStr);
	}
	//Load string for multi language
	//各文本的显示文字
	CString StringLoaded;
	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_TITLE"));
//	SetDlgItemText(IDC_SCH_LT_TITLE,StringLoaded);
	m_stLtTitle.SetWindowText(StringLoaded);
	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_STARTTIME"));
//	SetDlgItemText(IDC_SCH_LT_STARTTIME,StringLoaded);
	m_stStartTime.SetWindowText(StringLoaded);

	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_ENDTIME"));
	m_stStopTime.SetWindowText(StringLoaded);
	
	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_ALARM"));
//	SetDlgItemText(IDC_SCH_CHK_ALARM,StringLoaded);
	m_chkAlarm.SetWindowText(StringLoaded);
//	m_chkAlarm.SetWindowText((LPCSTR)StringLoaded);

 	StringLoaded = LoadStringFromFile(_T("public"),_T("IDS_CAL_ED_STARTTIME"));
	m_stStartTime.SetWindowText(StringLoaded);
 
	StringLoaded = LoadStringFromFile(_T("public"),_T("DLG_STATIC_REPEATENDDATA"));
	m_stRepeatEndData.SetWindowText(StringLoaded);

	//add for priority
	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_PRIORITY"));
	m_stPriority.SetWindowText(StringLoaded);

/*
	StringLoaded = LoadStringFromFile(_T("EditDlg"),"DLG_STATIC_ANOTHERTIME");
	m_cbAlarmLeadTime.AddString(StringLoaded);
	CString strmin = LoadStringFromFile(_T("EditDlg"),"DLG_STATIC_MIN");
	CString strHour = LoadStringFromFile(_T("EditDlg"),"DLG_STATIC_HOUR");
	CString strDay = LoadStringFromFile(_T("EditDlg"),"DLG_STATIC_DAY");

	int n = 0;
	for(i= 0 ; i <4; i++)
	{
		StringLoaded.Format(L"%d %s",n, strmin);
		m_cbAlarmLeadTime.AddString(StringLoaded);
		n +=5;
	}
	StringLoaded.Format(L"%d %s",30, strmin);
	m_cbAlarmLeadTime.AddString(StringLoaded);

	for(i= 1 ; i <=12; i++)
	{
		StringLoaded.Format(L"%d %s",i, strHour);
		m_cbAlarmLeadTime.AddString(StringLoaded);
	}
	StringLoaded.Format(L"%d %s",1, strDay);
	m_cbAlarmLeadTime.AddString(StringLoaded);
	StringLoaded.Format(L"%d %s",2, strDay);
	m_cbAlarmLeadTime.AddString(StringLoaded);
*/

	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_PHONENO"));
//	SetDlgItemText(IDC_SCH_LT_REMIND_PHONENO,StringLoaded);
	m_stAlarmPhoneNo.SetWindowText(StringLoaded);

	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_REPEAT"));
//	SetDlgItemText(IDC_SCH_CHK_REPEAT,StringLoaded);
	m_chkRepeat.SetWindowText(StringLoaded);
//	SetDlgItemText(IDC_SCH_GB_REPEAT_OPTION,StringLoaded);
//	GetDlgItem(IDC_SCH_GB_REPEAT_OPTION)->ShowWindow(SW_HIDE);

//	StringLoaded = LoadStringFromFile(_T("EditDlg"),"DLG_STATIC_REPEATEvery");
//	SetDlgItemText(IDC_SCH_LT_REPEAT_EVERY,StringLoaded);
/*
	//Load strings of repeat-type from profile
    for (int numRepeatType=0; numRepeatType<5; numRepeatType++)
	{
       	char szBuffer[10], szRepeatType[32];
    	itoa(numRepeatType, szBuffer, 10);
    	al_GetSettingString("repeattype", szBuffer, theApp.m_szRes,szRepeatType);
    	m_cbRepeatType.AddString(szRepeatType);
	}
*/


//	StringLoaded = LoadStringFromFile(_T("EditDlg"),"DLG_STATIC_INTERVALUNIT0");
//	SetDlgItemText(IDC_SCH_LT_REPEAT_INTERVALUNIT,StringLoaded);
	//循环方式组合框内容
	CString str;
	for (i=0;i<5;i++) 
	{
		str.Format(_T("DLG_STATIC_REPEAT_WHICHWEEK%d"),i);
		StringLoaded = LoadStringFromFile(_T("EditDlg"),str);
		m_cbRepeatWhichWeek.AddString(StringLoaded);
	}
	//优先级组合框内容        12.08 add for 9@9u
	CString str2;
	int idx;
	for (i=1;i<4;i++) 
	{
		str2.Format(_T("DLG_STATIC_PRIORITY%d"),i);
		StringLoaded = LoadStringFromFile(_T("EditDlg"),str2);
		idx = m_cbPriority.AddString(StringLoaded);
		m_cbPriority.SetItemData(idx, i);
	}
	idx = m_ScheduleInfo.uiPrivate;		//当编辑进度表时载入的信息在这里初始化
	//测试优先级的初始化
// 	CString strlll;
// 	strlll.Format(L"传递上来的uiPrivate : %d",idx);
// 	AfxMessageBox(strlll);
// 	if(idx == 1)
// 		m_cbPriority.SetCurSel(1);
// 	else
 		m_cbPriority.SetCurSel(idx - 1);


	//没用到
	for (i=0;i<7;i++) 
	{
		str.Format(_T("DLG_STATIC_REPEAT_REPEAT_WEEKDAY%d"),i);
		StringLoaded = LoadStringFromFile(_T("EditDlg"),str);
		m_cbWeekDay.AddString(StringLoaded);
	}

//	StringLoaded = LoadStringFromFile(_T("EditDlg"),"DLG_STATIC_UNIT");
//	SetDlgItemText(IDC_SCH_CHK_REPEAT_Until,StringLoaded);
	//几个按钮的文字
	StringLoaded = LoadStringFromFile(_T("public"),_T("Save"));
	SetDlgItemText(IDOK,StringLoaded);

	StringLoaded = LoadStringFromFile(_T("public"),_T("IDS_CANCEL"));
	SetDlgItemText(IDCANCEL,StringLoaded);

	StringLoaded = LoadStringFromFile(_T("public"),_T("MEMO"));
	SetDlgItemText(IDC_SCH_LT_MEMO,StringLoaded);

	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_TONE"));
	SetDlgItemText(IDC_SCH_CHK_TONE,StringLoaded);

	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_TYPE"));
	SetDlgItemText(IDC_SCH_LT_TYPE,StringLoaded);

	int StartTimeIndex = m_ScheduleInfo.rowIndex; //  xGetTimeIndex(m_ScheduleInfo.hiStartTime);
	m_CbStartTime.SetCurSel(StartTimeIndex);
	//重新根据开始时间添加结束时间 上面的添加无效
	xUpdateStopTimeCb(StartTimeIndex);
	//暂时没用到
	m_Title = m_ScheduleInfo.csTitle;
	m_edDescription.SetWindowText(m_ScheduleInfo.csMemo);

	//	m_StartDate = m_pMonthInfo->GetCurrentDate();
	m_StartDate = m_ScheduleInfo.dtStartDate;
	m_StopDate  = m_ScheduleInfo.dtStopDate ;


//	m_Alarmday = m_ScheduleInfo.dtStartDate;

	//Alarm  移动到checknotetype之后  kerm 2.15
 	//m_bAlarm = m_ScheduleInfo.haveAlarm;
 	//m_chkAlarm.SetCheck(m_bAlarm);

	
//	m_AlarmLeadTime = m_ScheduleInfo.alarmLeadtime;
/*	int nHour=0, nMin=0;
	m_ScheduleInfo.hiStartTime.Get24HourTime(nHour, nMin);

	COleDateTime dtStart(m_ScheduleInfo.dtStartDate.GetYear(),m_ScheduleInfo.dtStartDate.GetMonth(),m_ScheduleInfo.dtStartDate.GetDay(),
		nHour,nMin,0);
	DWORD dwMin = m_ScheduleInfo.GetAlarmBeforeMin(dtStart,m_ScheduleInfo.dtAlarmDateTime);

	
*/
	
	m_Alarmday = m_ScheduleInfo.dtAlarmDateTime;
	m_AlarmTime.SetTime(m_Alarmday.GetHour(), m_Alarmday.GetMinute(), m_Alarmday.GetSecond());


	//Alarm PhoneNo
	m_AlarmPhoneNo = m_ScheduleInfo.telnumber;

	//Tone
	m_bTone = m_ScheduleInfo.bhaveTone;
	m_ckTone.SetCheck(m_bTone);

	//Repeat
	m_bRepeat = m_ScheduleInfo.haveRepeat;
	m_chkRepeat.SetCheck(m_bRepeat);

	m_chkRepeatEndDate.EnableWindow(m_bRepeat);
	m_stRepeatEndData.EnableWindow(m_bRepeat);
	m_spRepeatinterval.EnableWindow(m_bRepeat);
	m_edRepeatInterval.EnableWindow(m_bRepeat);
	m_scRepeatinterval.EnableWindow(m_bRepeat);
	m_scRepeatON.EnableWindow(m_bRepeat);


//	m_RepeatType = m_ScheduleInfo.repeatType;
	m_WhichDay = m_ScheduleInfo.rfWhichDay;
	m_RepeatWhichWeek = m_ScheduleInfo.rfWhichWeek;
	m_nDayofWeekMask = m_ScheduleInfo.nDayofWeekMask;
	
	TCHAR szNum[MAX_PATH];
	_itot(m_ScheduleInfo.repeatFrequency,szNum,10);
	m_edRepeatInterval.SetWindowText(szNum);
	
	//Repeat until
	m_chkRepeatEndDate.SetCheck(m_ScheduleInfo.haveRepeatUntil);
//	m_bRepeatUntil = m_ScheduleInfo.haveRepeatUntil;
	if (m_ScheduleInfo.utilWhichDate._yy==0) 
	{
		m_UntilDate = m_StartDate;
	} 
	else 
	{
		m_UntilDate.SetDate(m_ScheduleInfo.utilWhichDate._yy,m_ScheduleInfo.utilWhichDate._mm,m_ScheduleInfo.utilWhichDate._dd);
	}

	//Place
	this->m_csLocation = (LPCTSTR) m_ScheduleInfo.cuLocation;


	//NoteType
	if (0 < m_ScheduleInfo.uiNodeType)
	{
    	//NoteType
		for (int nSize=0; nSize<m_ArrayofSupportNoteType.GetSize(); nSize++)
		{
			if (m_ArrayofSupportNoteType[nSize].nNodeType == m_ScheduleInfo.uiNodeType)
			{
            	this->m_cbIndex_NoteType = nSize;
			}
		}

	}
	else
	{
		m_ScheduleInfo.uiNodeType = m_ArrayofSupportNoteType[0].nNodeType;
        this->m_cbIndex_NoteType = 0;
	}

	CString strStart,strStop;
	strStart = m_ScheduleInfo.hiRealStartTime.GetAMPMTimeString();
	strStop = m_ScheduleInfo.hiRealStopTime.GetAMPMTimeString();
	
	

	int nStartEndTimeInterval = GetTimeInterval(strStart,strStop) / 30;

	m_CbStopTime.SetCurSel(nStartEndTimeInterval);

	m_CbStartTime.SetWindowText(strStart);

	m_CbStopTime.SetWindowText(strStop);

	COleDateTime dtStartDateTime,dtStopDateTime;
	int nStartHour,nStartMin,nStopHour,nStopMin;
	m_ScheduleInfo.hiRealStartTime.Get24HourTime(nStartHour,nStartMin);
	m_ScheduleInfo.hiRealStopTime.Get24HourTime(nStopHour,nStopMin);
	dtStartDateTime.SetDateTime(m_ScheduleInfo.dtStartDate.GetYear(),m_ScheduleInfo.dtStartDate.GetMonth(),m_ScheduleInfo.dtStartDate.GetDay(),nStartHour,nStartMin,0);
	dtStopDateTime.SetDateTime(m_ScheduleInfo.dtStopDate.GetYear(),m_ScheduleInfo.dtStopDate.GetMonth(),m_ScheduleInfo.dtStopDate.GetDay(),nStopHour,nStopMin,0);

	xUpdateData(FALSE);

	Check_NoteType();

	//如果主题不为空，则禁止改变类型 kerm add for 9a9u 1.10
	if(!m_ScheduleInfo.csTitle.IsEmpty())
		m_cbNoteType.EnableWindow(false);

	//在初始化时增加repeat对alarm的限制    kerm add for 9a9u 08.1.02
	//在读取数据时起作用，必须放在Check_NoteType()后面
	//去掉重复对闹铃的限制 kerm change 2.15
/*	if(m_ScheduleInfo.haveRepeat)
	{
		m_chkAlarm.EnableWindow(true);
		m_chkAlarm.SetCheck(m_ScheduleInfo.haveAlarm);
		m_cbAlarmLeadTime.EnableWindow(true);
	}*/

	//在读取数据时起作用，必须放在Check_NoteType()后面  kerm 2.15
	m_bAlarm = m_ScheduleInfo.haveAlarm;
	m_chkAlarm.SetCheck(m_bAlarm);
	
	SetStopIntervalValue(dtStartDateTime,dtStopDateTime);
	if(m_ScheduleInfo.dtAlarmDateTime.m_dt > 0)
	SetAlarmIntervalValue(dtStartDateTime,m_ScheduleInfo.dtAlarmDateTime);

	//Sanjeev modified on 20031212
	if(!m_AlarmPhoneNo.IsEmpty())
	{
		TCHAR strRet[1024];
		memset(strRet,0x00,1024);
		_tcscpy(strRet,m_AlarmPhoneNo);
		if(_tcslen(strRet) > 0)
        {
            CString strPhone;
            if(strRet[0] == 0x26)
			{
				strPhone = strRet+3;
				m_ExtraBytes = m_AlarmPhoneNo.Left(3);
			}
            if(strRet[0] == 0x25)
			{
				strPhone = strRet+1;
				m_ExtraBytes = m_AlarmPhoneNo.Left(1);
			}

            if(strRet[0] != 0x26 && strRet[0] != 0x25)
			{
				strPhone = strRet;
				m_ExtraBytes = _T("");
			}
            memset(strRet, 0x00, 1024);
            //strRet[0] = 0x25;
            _tcscpy(strRet, strPhone);
        }
		m_AlarmPhoneNo = strRet;
	}
	SetDlgItemText(IDC_SCH_DATA_ALARM_PHONENO,m_AlarmPhoneNo);
	//Sanjeev modified above lines on 20031212

	TCHAR ustr[MAX_PATH];
	al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes, ustr);
	SetWindowText((LPCTSTR)ustr);


	CWnd* pWnd = GetDlgItem(IDC_SCH_DATA_TITLE);
	PostMessage(WM_NEXTDLGCTL,(WPARAM)pWnd->GetSafeHwnd(),TRUE);


	//Nono, 2004_0613
	if( !(m_DialogTitle.IsEmpty()) ) 
	{
		SetWindowText(m_DialogTitle);
	}

	if(!m_bShowCtrlItem) 
	{
		ShowCtrlItem(FALSE);
	}

	//Calendar
	OnSchChkAlarm();
    //OnSelchangeAlarmInterval();

/*
	if (m_ScheduleInfo.hiRealStopTime.hour || m_ScheduleInfo.hiRealStopTime.min ) {
     	m_CbStopTime.SetWindowText(m_ScheduleInfo.hiRealStopTime.GetAMPMTimeString());
		m_StopTime = -1;
		m_CbStopTime.SetCurSel(2);
	}else {
    	m_StopTime =  m_ScheduleInfo.n30MinBlockCnt - 1; //xGetTimeIndex(m_ScheduleInfo.hiStopTime);
	}
*/
	m_etAlarmPhoneNo.SetNumFlag(TRUE);

	PostMessage(WM_CHECKVIEWMODE,0,0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCAMMScheduleEditorDlg::SetScheduleInfo(CScheduleInfo si) 
{
	if (si.csTitle.GetLength() > 0) 
	{
		m_bCreateNew = FALSE;
	}
	m_ScheduleInfo = si;
}

void CCAMMScheduleEditorDlg::GetScheduleInfo(CScheduleInfo& si)
 {
	si = m_ScheduleInfo;
}

int CCAMMScheduleEditorDlg::xGetTimeIndex(CScheduleTimeInfo sti) {
	//	int nRet = hi.hour;
	//	nRet += hi.halfDay==HALFDAY_AM?12:24;
	//	nRet -= 12;
	//	nRet *= 2;
	//	nRet += hi.min==0?0:1;
	//	return nRet;
	return (((sti.hour + (sti.halfDay==HALFDAY_AM?12:24) - 12) * 2) + (sti.min==0?0:1));
}

CScheduleTimeInfo CCAMMScheduleEditorDlg::xGetHourInfoByTimeIndex(int timeIndex) {
	//	CScheduleTimeInfo sti;
	//	sti.halfDay = timeIndex < 24 ? HALFDAY_AM : HALFDAY_PM;
	//	sti.hour = timeIndex / 2 + 12 - (sti.halfDay==HALFDAY_AM?12:24);
	//	sti.min = (timeIndex % 2 == 1? 30 : 0);
	//
	//	return sti;
	return m_pMonthInfo->GetInfo(timeIndex).hiStartTime;
}


void CCAMMScheduleEditorDlg::xUpdateData(BOOLEAN bControl2Variable)
{

	//	if (bControl2Variable) {
	//		xGetDlgItemText(IDC_SCH_DATA_TITLE,m_Title,MAX_LENGTH);
	//		xGetDlgItemText(IDC_SCH_DATA_MEMO,m_Memo,MAX_LENGTH);
	//		xGetDlgItemText(IDC_SCH_DATA_ALARM_PHONENO,m_AlarmPhoneNo,MAX_LENGTH);
	//		xGetDlgItemText(IDC_SCH_DATA_REPEAT_INTERVAL,m_RepeatFrequency,MAX_LENGTH);
	//	} else {
	//		SetDlgItemText(IDC_SCH_DATA_TITLE,m_Title);
	//		SetDlgItemText(IDC_SCH_DATA_MEMO,m_Memo);
	//		SetDlgItemText(IDC_SCH_DATA_ALARM_PHONENO,m_AlarmPhoneNo);
	//		SetDlgItemText(IDC_SCH_DATA_REPEAT_INTERVAL,m_RepeatFrequency);
	//	}
	UpdateData(bControl2Variable);


}

void CCAMMScheduleEditorDlg::xGetDlgItemText(INT ControlId, CString& ExchangeString, INT MaxLength)
{

	TCHAR *lStr = new TCHAR[MaxLength];
	memset(lStr,0x00,sizeof(TCHAR)*MaxLength);
	GetDlgItemText(ControlId,lStr,MaxLength);
	ExchangeString = lStr;

	delete [] lStr;


}

void CCAMMScheduleEditorDlg::OnOK() 
{
	// TODO: Add extra validation here
	//在list view中 设置只读模式，在此情况下点击ok直接退出？
	if(m_bViewOnly) 
	{
		CDialog::OnOK();
		return ;
	}

	xUpdateData(TRUE);
	CString Msg;

   	CString csCtrlText;
	//根据当前选择的combo的内容确定notetype
	int nNoteType = m_ArrayofSupportNoteType[m_cbIndex_NoteType].nNodeType;

	//检查主题是不是为空
	if (m_Title.IsEmpty()) 
	{		
		CString str = LoadStringFromFile(_T("public"),_T("IDS_CAL_MSG_EMPTY_TEXT1"));
		CString str2 = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText1ID);
		csCtrlText.Format(str,str2);
	//	AfxMessageBox(csCtrlText);
		//如果为空，弹出提示，并将置为焦点
		CalendarStringMessageBox(m_hWnd,csCtrlText);
		GetDlgItem(IDC_SCH_DATA_TITLE)->SetFocus();
		return;
	}
 
	//得到当前calendar的支持类型 存入supportinfo中
	CalendarSupportInfo SupportInfo;
	CCAMMSync sc;
	sc.GetCalendarSupport(SupportInfo);

	CString cuLocation = m_csLocation;
	int n = m_Title.GetLength();
	CString strMsg;
	//当该calendar的主题、描述、地点 都是空时不成立
	if(SupportInfo.Title_Boundry > 0 || SupportInfo.Location_Boundry >0 || SupportInfo.Description_Boundry >0)
	{
		int nTitleBytes = SupportInfo.Title_Boundry;
		int nLocationBytes = SupportInfo.Location_Boundry ;
		int nDecriptionBytes = SupportInfo.Description_Boundry ;

		CString strErr;
#ifdef _UNICODE
		if(SupportInfo.CharType == 1)	//chartype ?? 由底层返回
		{	
			//？？
			USES_CONVERSION;
			int nLen = m_Title.GetLength();

			char* pszText = new char[(nLen+1)*2];
			sprintf(pszText,"%s",W2A(m_Title));		//将主题存入pszText

			int nAnsiTextLen = strlen(pszText);
			//判断是否超出主题的允许字符数
			if(nTitleBytes > 0 && nAnsiTextLen > nTitleBytes)
			{	
				//从ini中读取超出字符数消息的内容
				strMsg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_EXCEED_LENGTH"));
				csCtrlText.Format(strMsg,SupportInfo.Title_Boundry);

				CString strText = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText1ID);
				strErr.Format(_T("%s :\n%s"),strText,csCtrlText);
				//弹出对话框，并置为焦点
				CalendarStringMessageBox(m_hWnd,strErr);
				GetDlgItem(IDC_SCH_DATA_TITLE)->SetFocus();
				delete []pszText;
				return;
			}
			delete[] pszText;

			nLen = m_csLocation.GetLength();
			char* pszLocation = new char[(nLen+1)*2];
			sprintf(pszLocation,"%s",W2A(m_csLocation));

			nAnsiTextLen = strlen(pszLocation);
			//检查是否地点输入超出字符数限制
			if(nLocationBytes > 0 && nAnsiTextLen > nLocationBytes)
			{
				strMsg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_EXCEED_LENGTH"));
				csCtrlText.Format(strMsg,SupportInfo.Location_Boundry);

				CString strText = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText2ID);
				strErr.Format(_T("%s :\n\n%s"),strText,csCtrlText);
				//弹出对话框，并置焦点
				CalendarStringMessageBox(m_hWnd,strErr);
				m_etLocation.SetFocus();
				delete []pszLocation;
				return;
			}
			delete []pszLocation;
			
			CString strDescription ;
			m_edDescription.GetWindowText(strDescription);

			nLen = strDescription.GetLength();
			char* pszDescription = new char[(nLen+1)*2];
			sprintf(pszDescription,"%s",W2A(strDescription));

			nAnsiTextLen = strlen(pszDescription);
			//检查描述输入是否超出字符限制
			if(nDecriptionBytes > 0 && nAnsiTextLen > nDecriptionBytes)
			{
				strMsg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_EXCEED_LENGTH"));
				csCtrlText.Format(strMsg,SupportInfo.Description_Boundry);

				CString strText = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText3ID);
				strErr.Format(_T("%s :\n\n%s"),strText,csCtrlText);
				
				CalendarStringMessageBox(m_hWnd,strErr);
				m_edDescription.SetFocus();
				delete []pszDescription;
				return;
			}
			delete []pszDescription;

		}	
		else	//当chartype不是1
		{
			//当主题文字长度超出字符数限制
			if(nTitleBytes > 0 && wcslen(m_Title) > nTitleBytes)
			{
				strMsg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_EXCEED_LENGTH"));
				csCtrlText.Format(strMsg,SupportInfo.Title_Boundry);

				CString strText = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText1ID);
				strErr.Format(_T("%s :\n%s"),strText,csCtrlText);
				
				CalendarStringMessageBox(m_hWnd,strErr);
				GetDlgItem(IDC_SCH_DATA_TITLE)->SetFocus();
				return;
			}
			//地点超出字符数限制
			if(nLocationBytes > 0 && wcslen(m_csLocation) > nLocationBytes)
			{
				strMsg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_EXCEED_LENGTH"));
				csCtrlText.Format(strMsg,SupportInfo.Location_Boundry);

				CString strText = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText2ID);
				strErr.Format(_T("%s :\n\n%s"),strText,csCtrlText);
				
				CalendarStringMessageBox(m_hWnd,strErr);
				m_etLocation.SetFocus();
				return;
			}
			CString strDescription ;	//描述 暂时没有用到 为什么跟上面不同？
			m_edDescription.GetWindowText(strDescription);
			if(nDecriptionBytes > 0 && wcslen(strDescription) > nDecriptionBytes)
			{
				strMsg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_EXCEED_LENGTH"));
				csCtrlText.Format(strMsg,SupportInfo.Description_Boundry);

				CString strText = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText3ID);
				strErr.Format(_T("%s :\n\n%s"),strText,csCtrlText);
				
				CalendarStringMessageBox(m_hWnd,strErr);
				m_edDescription.SetFocus();
				return;
			}
		}
#else
//下面是非unicode环境下的代码 跟上面类似
		if(SupportInfo.CharType == 1)
		{
			if(nTitleBytes > 0 && m_Title.GetLength() > nTitleBytes)
			{
				strMsg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_EXCEED_LENGTH"));
				csCtrlText.Format(strMsg,SupportInfo.Title_Boundry);

				CString strText = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText1ID);
				strErr.Format(_T("%s :\n%s"),strText,csCtrlText);
				
				CalendarStringMessageBox(m_hWnd,strErr);
				GetDlgItem(IDC_SCH_DATA_TITLE)->SetFocus();
				return;
			}
			if(nLocationBytes > 0 && m_csLocation.GetLength() > nLocationBytes)
			{
		//		if(GSM_CAL_CALL == m_cbIndex_NoteType+1)
	//			if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].bPhoneNoText2)
	//				strMsg = LoadStringFromFile("message","MSG_ERROR_PHONENO_EXCEED_LENGTH");
	//			else
	//				strMsg = LoadStringFromFile("message","MSG_ERROR_LOCATION_EXCEED_LENGTH");

	//			csCtrlText.Format(strMsg,SupportInfo.Location_Boundry);
 	//			CalendarStringMessageBox(m_hWnd,csCtrlText);
				strMsg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_EXCEED_LENGTH"));
				csCtrlText.Format(strMsg,SupportInfo.Location_Boundry);

				CString strText = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText2ID);
				strErr.Format(_T("%s :\n\n%s"),strText,csCtrlText);
				
				CalendarStringMessageBox(m_hWnd,strErr);
				m_etLocation.SetFocus();
				return;
			}
			CString strDescription ;
			m_edDescription.GetWindowText(strDescription);
			if(nDecriptionBytes > 0 && strDescription.GetLength() > nDecriptionBytes)
			{
	//			strMsg = LoadStringFromFile("message","MSG_ERROR_DESCRIPTION_EXCEED_LENGTH");
	//			csCtrlText.Format(strMsg,SupportInfo.Description_Boundry);
	//			CalendarStringMessageBox(m_hWnd,csCtrlText);
				strMsg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_EXCEED_LENGTH"));
				csCtrlText.Format(strMsg,SupportInfo.Description_Boundry);

				CString strText = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText3ID);
				strErr.Format(_T("%s :\n\n%s"),strText,csCtrlText);
				
				CalendarStringMessageBox(m_hWnd,strErr);
				m_edDescription.SetFocus();
				return;
			}

		}
		else
		{
			int nTitleBytes = SupportInfo.Title_Boundry;
			int nLocationBytes = SupportInfo.Location_Boundry ;
			int nDecriptionBytes = SupportInfo.Description_Boundry ;
			if(nTitleBytes > 0 && m_Title.GetLength()>0)
			{
				int nLength = m_Title.GetLength() *2;
				WCHAR	*pszTextW = new WCHAR[nLength];

				MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_Title, -1,
									pszTextW, nLength);

				if(wcslen(pszTextW) > nTitleBytes)
				{
		//			strMsg = LoadStringFromFile("message","MSG_ERROR_SUBJECT_EXCEED_LENGTH");
		//			csCtrlText.Format(strMsg,SupportInfo.Title_Boundry);
		//			CalendarStringMessageBox(m_hWnd,csCtrlText);

					strMsg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_EXCEED_LENGTH"));
					csCtrlText.Format(strMsg,SupportInfo.Title_Boundry);

					CString strText = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText1ID);
					strErr.Format(_T("%s :\n\n%s"),strText,csCtrlText);
					
					CalendarStringMessageBox(m_hWnd,strErr);
					GetDlgItem(IDC_SCH_DATA_TITLE)->SetFocus();

					delete pszTextW;
					return;
				}

				delete pszTextW;
			}
			if(nLocationBytes > 0 && m_csLocation.GetLength() >0)
			{
				int nLength = m_csLocation.GetLength() *2;
				WCHAR	*pszLocationW = new WCHAR[nLength];

				MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_csLocation, -1,
									pszLocationW, nLength);

				if(wcslen(pszLocationW) > nLocationBytes)
				{
		/*			if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].bPhoneNoText2)
						strMsg = LoadStringFromFile("message","MSG_ERROR_PHONENO_EXCEED_LENGTH");
					else
						strMsg = LoadStringFromFile("message","MSG_ERROR_LOCATION_EXCEED_LENGTH");

					csCtrlText.Format(strMsg,SupportInfo.Location_Boundry);
 					CalendarStringMessageBox(m_hWnd,csCtrlText);*/
					strMsg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_EXCEED_LENGTH"));
					csCtrlText.Format(strMsg,SupportInfo.Location_Boundry);

					CString strText = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText2ID);
					strErr.Format(_T("%s :\n\n%s"),strText,csCtrlText);
					
					CalendarStringMessageBox(m_hWnd,strErr);
					m_etLocation.SetFocus();
					delete pszLocationW;
					return;
				}

				delete pszLocationW;
			}
			if(nDecriptionBytes > 0)
			{
				CString strDescription ;
				m_edDescription.GetWindowText(strDescription);
				int nLength = strDescription.GetLength() *2;
				if(nLength > 0)
				{
					WCHAR	*pszDescriptionW = new WCHAR[nLength];

					MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strDescription, -1,
										pszDescriptionW, nLength);

					if(wcslen(pszDescriptionW) > nDecriptionBytes)
					{
		//				strMsg = LoadStringFromFile("message","MSG_ERROR_DESCRIPTION_EXCEED_LENGTH");
		//				csCtrlText.Format(strMsg,SupportInfo.Description_Boundry);
		//				CalendarStringMessageBox(m_hWnd,csCtrlText);
						strMsg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_EXCEED_LENGTH"));
						csCtrlText.Format(strMsg,SupportInfo.Description_Boundry);

						CString strText = LoadStringFromFile(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText3ID);
						strErr.Format(_T("%s :\n\n%s"),strText,csCtrlText);
						
						CalendarStringMessageBox(m_hWnd,strErr);
						delete pszDescriptionW;
						m_edDescription.SetFocus();
						return;
					}

					delete pszDescriptionW;
				}
			}
		}	//非uncode环境结束
#endif

	/*		int nTitleBytes = SupportInfo.Title_Boundry;
			int nLocationBytes = SupportInfo.Location_Boundry ;
			int nDecriptionBytes = SupportInfo.Description_Boundry ;
			WCHAR				szLocationW[MAX_LENGTH*2];
			WCHAR				szTextW[MAX_LENGTH*2];
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_csLocation, -1,
								szLocationW, MAX_LENGTH*2);
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_Title, -1,
								szTextW, MAX_LENGTH*2);
			int a = m_Title.GetLength();
			int nn =wcslen(szTextW);
			if(nTitleBytes > 0 && wcslen(szTextW) > nTitleBytes)
			{
				strMsg = LoadStringFromFile("message","MSG_ERROR_SUBJECT_EXCEED_LENGTH");
				csCtrlText.Format(strMsg,SupportInfo.Title_Boundry);
				CalendarStringMessageBox(m_hWnd,csCtrlText);
				return;
			}
			if(nLocationBytes > 0 && wcslen(szLocationW) > nLocationBytes)
			{
		//		if(GSM_CAL_CALL == m_cbIndex_NoteType+1)
				if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].bPhoneNoText2)
					strMsg = LoadStringFromFile("message","MSG_ERROR_PHONENO_EXCEED_LENGTH");
				else
					strMsg = LoadStringFromFile("message","MSG_ERROR_LOCATION_EXCEED_LENGTH");

				csCtrlText.Format(strMsg,SupportInfo.Location_Boundry);
 				CalendarStringMessageBox(m_hWnd,csCtrlText);
				return;
			}
		}*/
	}
	
	//开始与结束时间 
	TCHAR txt[MAX_LENGTH];
	CScheduleTimeInfo stiStartTime;
	CScheduleTimeInfo stiStopTime;
	memset(txt,0x00,MAX_LENGTH);
	m_CbStartTime.GetWindowText(txt,MAX_LENGTH);	

	CString txtStr=txt;								
	if((m_CbStartTime.GetStyle() & WS_VISIBLE))		
	{
		if (!stiStartTime.ParseAMPMTimeString(txtStr))	//将得到的开始时间格式化 放入stiStartTime
		{
			CalendarMessageBox(m_hWnd,_T("MSG_ERROR_TIMEFORMAT"));
			return;
		}
	}

	memset(txt,0x00,MAX_LENGTH);	//得到结束时间 并格式化 存入stiStopTime
	m_CbStopTime.GetWindowText(txt,MAX_LENGTH);
	txtStr=txt;
	if((m_CbStopTime.GetStyle() & WS_VISIBLE))
	{
		if (!stiStopTime.ParseAMPMTimeString(txtStr)) 
		{
			CalendarMessageBox(m_hWnd,_T("MSG_ERROR_TIMEFORMAT"));
			return;
		}
	}

	//Check the stoptime	检查停止时间的合法性
	if( IsStopDateTimelegal() == FALSE)
	{
		CalendarMessageBox(m_hWnd,_T("MSG_ERROR_ENDTIME"));
		return;
	}
	//闹铃 状态
	if(m_bAlarm && (m_chkAlarm.GetStyle() & WS_VISIBLE))
	{
		int nHour,nMin;
		//如果有起始时间组合框，将开始时间的 时 分 付给nHour nMin 
		if((m_CbStartTime.GetStyle() & WS_VISIBLE))
		{
			TCHAR txt[MAX_LENGTH];
			memset(txt,0x00,MAX_LENGTH);
			m_CbStartTime.GetWindowText(txt,MAX_LENGTH);

			CString txtStr=txt;
			CScheduleTimeInfo startTimeInfo;
			startTimeInfo.ParseAMPMTimeString(txtStr);	//分析am pm 
			startTimeInfo.Get24HourTime(nHour,nMin);	//得到24小时制
		}
		else
		{
			nHour = 9 ;
			nMin = 0;
		}

		BOOL bTimeOnly = FALSE;
		COleDateTime dtAlarm,dtStart;
		dtStart.SetDateTime(m_StartDate.GetYear(), m_StartDate.GetMonth(), m_StartDate.GetDay(),nHour,nMin,0);
		if(GetAlarmDateTime(dtStart ,dtAlarm,bTimeOnly))	//根据starttime 与 leadtime 计算出alsrmtime
		{
		
			if((m_CbStartTime.GetStyle() & WS_VISIBLE))
			{
				if(dtAlarm > dtStart)	//闹铃时间不能大于开始时间
				{
					CalendarMessageBox(m_hWnd,_T("MSG_ERROR_ALARMTIME"));
					return;
				}
			}
			else	//下面代码一般得不到执行
			{
				if(bTimeOnly == FALSE)
				{
					if(dtAlarm.GetYear() > dtStart.GetYear() ||
						dtAlarm.GetMonth() > dtStart.GetMonth() ||
						dtAlarm.GetDay() > dtStart.GetDay() )
					{
						CalendarMessageBox(m_hWnd,_T("MSG_ERROR_ALARMTIME"));
						return;
					}

				}
			}
		}
/*		if(GetAlarmLeadTime() == 65535)
		{
			COleDateTime dtAlarm,dtStart;
			int nY,nM,nD;
			nY = m_Alarmday.GetYear();
			nM = m_Alarmday.GetMonth();
			nD = m_Alarmday.GetDay();
			dtAlarm.SetDateTime(nY,nM,nD,m_AlarmTime.GetHour(), m_AlarmTime.GetMinute(), m_AlarmTime.GetSecond());

			TCHAR txt[MAX_LENGTH];
			memset(txt,0x00,MAX_LENGTH);
			m_CbStartTime.GetWindowText(txt,MAX_LENGTH);

			int nHour,nMin;
			CString txtStr=txt;
			CScheduleTimeInfo startTimeInfo;
			startTimeInfo.ParseAMPMTimeString(txtStr);
			startTimeInfo.Get24HourTime(nHour,nMin);

			dtStart.SetDateTime(m_StartDate.GetYear(), m_StartDate.GetMonth(), m_StartDate.GetDay(),nHour,nMin,0);
			if(dtAlarm > dtStart)
			{
				CalendarMessageBox(m_hWnd,"MSG_ERROR_ALARMTIME");
				return;
			}
		
		}*/
	}
	//Check the UntilDate 重复报铃
	//重复到哪天 
	if(GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE)->IsWindowEnabled())
	{
		if((m_chkRepeatEndDate.GetStyle() & WS_VISIBLE))
		{
			COleDateTime dttempStopDate;
			dttempStopDate.SetDateTime(m_StopDate.GetYear(),m_StopDate.GetMonth(),m_StopDate.GetDay(),0,0,0);
			//暂时没有用到选择按钮，此处的重复到根据闹铃是否开启自动变化
			if (m_bRepeat && m_chkRepeatEndDate.GetCheck()) 
			{
				if (dttempStopDate >= m_UntilDate) {
					CalendarMessageBox(m_hWnd,_T("MSG_ERROR_REPEATENDDATE"));
					return;
				}
			}
		}
		else if(GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE)->GetStyle() & WS_VISIBLE)
		{
			//9@9u使用了 重复到 组合框
			COleDateTime dttempStopDate;
			dttempStopDate.SetDateTime(m_StopDate.GetYear(),m_StopDate.GetMonth(),m_StopDate.GetDay(),0,0,0);
			if (m_bRepeat) 
			{
				if (dttempStopDate >= m_UntilDate)
				{
					CalendarMessageBox(m_hWnd,_T("MSG_ERROR_REPEATENDDATE"));
					return;
				}
			}
		}
	}

	//是否内容改变
	if (xIsDataModified())	//代码中增加了对priority组合框的判断
	{
		if (m_bCreateNew) 
		{
			if (!xGenerateSchedule())	//重要的一个函数 调用这个函数向底层赋值。
			{
				return;
			}
		} 
		else 
		{
			int OKFlag = CalendarMessageBox(m_hWnd,_T("MSG_REPLACE_DATA"),MB_YESNO);
			if (OKFlag == IDNO) 
			{
				return;
			} 
			else 
			{
				if (!xGenerateSchedule()) 
				{
					return;
				}
			}
		}
	}
	else 
	{
		CDialog::OnCancel();
		return;
	}


	CDialog::OnOK();
}

BOOL CCAMMScheduleEditorDlg::xGenerateSchedule() 
{


	int orgRow = m_ScheduleInfo.rowIndex;
	//得到当前calendar的类型
	m_ScheduleInfo.uiNodeType = m_ArrayofSupportNoteType[m_cbIndex_NoteType].nNodeType;

	// for test
// 	CString strtest;
// 	strtest.Format(L"监测 => m_ScheduleInfo.uiNodeType\n返回值 %d ", m_ScheduleInfo.uiNodeType);
// 	AfxMessageBox(strtest);

	CScheduleInfo si;
	//？为什么在这里就设置了
	si.SetScheduleInfoData(m_ScheduleInfo);

//	si.dtStartDate = m_StartDate;

	si.bScheduleStart = TRUE;

	TCHAR txt[MAX_LENGTH];
	memset(txt,0x00,MAX_LENGTH);
	//得到起始时间（不是日期）
	m_CbStartTime.GetWindowText(txt,MAX_LENGTH);

	CString txtStr=txt;
	CScheduleTimeInfo startTimeInfo,stopTimeInfo,AM900TimeInfo;
	startTimeInfo.ParseAMPMTimeString(txtStr);	//解析txtStr得到的时间 主要分析am 还是 pm 还有具体的时间

	BOOL bGetStopDateTime = FALSE;	//干什么用的？
	  COleDateTime dtStop;
	  //得到结束的时间，
	  if(GetStopDateTime(dtStop))
	  {
		  bGetStopDateTime = TRUE;	//如果上面函数调用成功，修改
		  stopTimeInfo.SetData(dtStop.GetHour(),dtStop.GetMinute(),dtStop.GetSecond());
	  }
	  else	//如果没有成功，分析结束时间组合框，不过上边不可能失败....
	  {
		memset(txt,0x00,MAX_LENGTH);
		m_CbStopTime.GetWindowText(txt,MAX_LENGTH);
		txtStr=txt;
		stopTimeInfo.ParseAMPMTimeString(txtStr);
	  }
	//这句应该没用
	AM900TimeInfo.SetData(9,0,0);

	//Alarm
	CScheduleTimeInfo AlarmTimeInfo;
	si.haveAlarm = m_bAlarm;

	if (m_bAlarm && (m_chkAlarm.GetStyle() & WS_VISIBLE))
	{
		
		COleDateTime dtStart;
		int nHour,nMin;
		BOOL bTimeOnly = FALSE;
		//在这里得到完整的起始日期 时间 赋给dtStart
		startTimeInfo.Get24HourTime(nHour,nMin);
		dtStart.SetDateTime(m_StartDate.GetYear(), m_StartDate.GetMonth(), m_StartDate.GetDay(),nHour,nMin,0);
		//下面函数通过起始时间与报警提前时间计算出 si.dtAlarmDateTime 通过引用返回的 包括日期时间
		GetAlarmDateTime(dtStart ,si.dtAlarmDateTime,bTimeOnly);
		
		//		si.alarmLeadtime = m_AlarmLeadTime;
	//	if(m_AlarmLeadTime !=0)
	//		si.SetAlarmLeadDatetime();
	//	else
/*		{
			int nY,nM,nD,nH,nMin,nS;
			if(m_cbAlarmday.IsWindowVisible())
			{
				nY = m_Alarmday.GetYear();
				nM = m_Alarmday.GetMonth();
				nD = m_Alarmday.GetDay();
			}
			else
			{
				nY = m_StartDate.GetYear();
				nM = m_StartDate.GetMonth();
				nD = m_StartDate.GetDay();
			}

			if(m_cbAlarmTime.IsWindowVisible())
			{
				nH =m_AlarmTime.GetHour() ;
				nMin =m_AlarmTime.GetMinute(); 
				nS =m_AlarmTime.GetSecond();
			}
			else
			{
				startTimeInfo.Get24HourTime(nH,nMin);
				nS =0;
			}
			si.dtAlarmDateTime.SetDateTime(nY,nM,nD,nH,nMin, nS);

		}*/
		//？？
		AlarmTimeInfo.SetData(si.dtAlarmDateTime.GetHour(), si.dtAlarmDateTime.GetMinute(), si.dtAlarmDateTime.GetSecond());

	
	}


	si.bEndDateSameToStartDate = TRUE;
	if((m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000008) ) //startTime
	{
		si.hiRealStartTime =startTimeInfo;	
		si.hiRealStopTime = stopTimeInfo;
	}
	else
	{ 
		if(m_bAlarm && (m_chkAlarm.GetStyle() & WS_VISIBLE))
		{
			si.hiRealStartTime = AlarmTimeInfo;
			si.hiRealStopTime = AlarmTimeInfo;
		}
		else
		{
			si.hiRealStartTime =AM900TimeInfo;	
			si.hiRealStopTime = AM900TimeInfo;
		}
		
	}

	si.hiStartTime = si.hiRealStartTime;	//向底层返回开始时间
	si.rowIndex = si.hiStartTime.GetHalfHourCount();
	// for test
// 	strtest.Format(L"监测 => si.hiStartTime.GetHalfHourCount()\n返回值 %d ", si.hiStartTime.GetHalfHourCount());
// 	AfxMessageBox(strtest);

	si.hiStopTime = si.hiRealStopTime;		//返回结束时间
//	si.hiStopTime.MoveToNearestStopTime();
  
	//for test
	//CString strtest;
// 	strtest.Format(L"监测 => si.hiStopTime.GetHalfHourCount()\n返回值 %d ", si.hiStopTime.GetHalfHourCount());
// 	AfxMessageBox(strtest);


	si.n30MinBlockCnt = si.hiStopTime.GetHalfHourCount() - si.hiStartTime.GetHalfHourCount();

	int nHour=0, nMin=0;
	si.hiStartTime.Get24HourTime(nHour, nMin);

	//for test
// 	CString strtest;
//  	strtest.Format(L"监测 => si.hiStartTime.Get24HourTime(nHour, nMin)\n返回值nHour=%d, nMin=%d ", nHour, nMin);
//  	AfxMessageBox(strtest);


	//返回开始日期
	si.dtStartDate.SetDateTime(m_StartDate.GetYear(), m_StartDate.GetMonth(), m_StartDate.GetDay(),nHour,nMin,0);

	si.hiStopTime.Get24HourTime(nHour, nMin);

	//for test
// 	strtest.Format(L"监测 => si.hiStopTime.Get24HourTime(nHour, nMin)\n返回值nHour=%d, nMin=%d ", nHour, nMin);
// 	AfxMessageBox(strtest);


//	if((m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000010)) //StopDate
	//返回结束日期
	if(bGetStopDateTime)
	{
		si.dtStopDate.SetDateTime(dtStop.GetYear(), dtStop.GetMonth(), dtStop.GetDay(),dtStop.GetHour(),dtStop.GetMinute(),dtStop.GetSecond());
		if( si.dtStartDate.GetYear() != si.dtStopDate.GetYear() ||
			 si.dtStartDate.GetMonth() != si.dtStopDate.GetMonth() ||
			 si.dtStartDate.GetDay() != si.dtStopDate.GetDay() )
		si.bEndDateSameToStartDate = FALSE;
	}
	else
	{
		if((m_cbStopDate.GetStyle() & WS_VISIBLE) && (m_cbStopDate.GetStyle() & WS_VISIBLE))
		{
			si.dtStopDate.SetDateTime(m_StopDate.GetYear(), m_StopDate.GetMonth(), m_StopDate.GetDay(),nHour,nMin,0);
			if( si.dtStartDate.GetYear() != si.dtStopDate.GetYear() ||
				 si.dtStartDate.GetMonth() != si.dtStopDate.GetMonth() ||
				 si.dtStartDate.GetDay() != si.dtStopDate.GetDay() )
			si.bEndDateSameToStartDate = FALSE;
		}
		else
			si.dtStopDate.SetDateTime(m_StartDate.GetYear(), m_StartDate.GetMonth(), m_StartDate.GetDay(),nHour,nMin,0);

	}
	si.csTitle = m_Title;	//返回主题

	//for test
// 	strtest.Format(L"监测 => si.csTitle\n返回值 %s", si.csTitle);
// 	AfxMessageBox(strtest);


	//下面是返回描述，没用用到
	if(m_edDescription.GetStyle() & WS_VISIBLE)
	{
		m_edDescription.GetWindowText(si.csMemo);
	}
	else
		si.csMemo = _T("");

	//NoteType
    si.uiNodeType = (unsigned int)  m_ArrayofSupportNoteType[m_cbIndex_NoteType].nNodeType;
	//for test
// 	CString strtest;
// 	strtest.Format(L"监测 => si.uiNodeType\n返回值 %d", si.uiNodeType);
// 	AfxMessageBox(strtest);

	//PhoneNumber for NoteType "CALL"	注意这里 
    si.telnumber = m_ExtraBytes+m_AlarmPhoneNo;

    //Location
   	CString csLocation(_T(""));
   	CString cuLocation(_T(""));
   	m_etLocation.GetWindowText(csLocation);
   	cuLocation = csLocation;
    si.cuLocation = cuLocation;

	//for test
// 	strtest.Format(L"监测 => si.cuLocation\n返回值 %s", si.cuLocation);
// 	AfxMessageBox(strtest);

	//Tone 没用到
	si.bhaveTone = m_bTone;

	//Repeat	
	si.haveRepeat = m_bRepeat;

	//增加对提醒类型的处理，当开始结束时间不是一天时，取消重复 kerm add for 9a9u 1.11
	if(si.uiNodeType == 1 && si.dtStartDate.GetDay() != si.dtStopDate.GetDay())	//1表示remainder类型
	{
		si.haveRepeat = m_bRepeat = false;
	}

	//单选按钮  重复  	
	if (m_bRepeat && (m_chkRepeat.GetStyle() & WS_VISIBLE))  //)//
	{
		CString str;
	//	str = m_RepeatFrequency;
	//	si.repeatFrequency = atoi(str);
		si.repeatType = m_cbRepeatType.GetItemData(m_cbRepeatType.GetCurSel());
	

//  		CString strdd;
//  		strdd.Format(L"在返回之前看看 repeattyep %d", si.repeatType);
//  		AfxMessageBox(strdd);

		if(si.repeatType == TWOWEEKLY)
			si.repeatFrequency =2;
		else
		{
			if((m_edRepeatInterval.GetStyle() & WS_VISIBLE))	//暂时不用 m_edRepeatInterval
			{
				TCHAR szrepeatFrequency[MAX_PATH];
				m_edRepeatInterval.GetWindowText(szrepeatFrequency,MAX_PATH);
				si.repeatFrequency = _ttoi(szrepeatFrequency);

			}
			else
				si.repeatFrequency =1;
		}

		//for test
// 		CString strtest;
// 		strtest.Format(L"监测 => si.repeatType\n返回值 %d", si.repeatType);
// 		AfxMessageBox(strtest);
		
		if(si.repeatType == WEEKDAYS )	//修改条件，重复为 WEEKDAYS 时出现复选
		{
			//修改条件 去掉 if
			//if(m_chWeek1.GetStyle() & WS_VISIBLE)
			//{
				int DayofWeekMask = 0;
 				if(m_chWeek1.GetCheck()) DayofWeekMask |=1;	

				if(m_chWeek2.GetCheck()) DayofWeekMask |=2;

				if(m_chWeek3.GetCheck()) DayofWeekMask |=4;

				if(m_chWeek4.GetCheck()) DayofWeekMask |=8;

				if(m_chWeek5.GetCheck()) DayofWeekMask |=16;

				if(m_chWeek6.GetCheck()) DayofWeekMask |=32;

				if(m_chWeek7.GetCheck()) DayofWeekMask |=64;

				si.nDayofWeekMask = DayofWeekMask;


				//如果全部取消，变成没有循环的方式   kerm add for 9a9u 1.05
				if(!m_chWeek1.GetCheck() && 
					!m_chWeek2.GetCheck() &&
					!m_chWeek3.GetCheck() &&
					!m_chWeek4.GetCheck() &&
					!m_chWeek5.GetCheck() &&
					!m_chWeek6.GetCheck() &&
					!m_chWeek7.GetCheck())
				{
					si.nDayofWeekMask = 0;
					si.repeatType = 1;
					si.haveRepeat = false;
					si.haveAlarm = false;
				}


		}


	//	si.rfWhichDay = m_WhichDay;
//		si.rfWhichWeek = m_RepeatWhichWeek;
		if (si.repeatType == WEEKLYMONTHLY)
			GetwhichWeekDay(si.dtStartDate,si.rfWhichWeek,si.rfWhichDay);
	}
	else//080613libaoliu
	{
     si.repeatType=-1;
	}



	//Repeat until 
	if((m_chkRepeatEndDate.GetStyle() & WS_VISIBLE))
	{
	//	si.haveRepeatUntil = m_bRepeatUntil;
		si.haveRepeatUntil = m_chkRepeatEndDate.GetCheck();
	//	if (m_bRepeatUntil) {
		if(si.haveRepeatUntil){
			si.utilWhichDate = xGetScheduleDate(m_UntilDate);
		}
	}
	else if(GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE)->GetStyle() & WS_VISIBLE)
	{
		if(GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE)->IsWindowEnabled())
		{
			si.haveRepeatUntil = 1;
			si.utilWhichDate = xGetScheduleDate(m_UntilDate);
		}
	}

	//Private priority
	si.uiPrivate = m_cbPriority.GetItemData(m_cbPriority.GetCurSel());

	//for test
// 	CString strtest;
// 	strtest.Format(L"si.dayofweek=%d\nsi.repeat type=%d", si.nDayofWeekMask, si.repeatType);
// 	AfxMessageBox(strtest);

	//supply StartTime into dtStartDate
	

	//supply StopTime into dtStartDate

	
	// ** Calendar ** //
	//下面函数向底层返回
	m_ScheduleInfo.SetScheduleInfoData(si);
	
	//Dump the content of calendar created
    m_ScheduleInfo.DumpScheduleConntent();	


	return TRUE;
}

void CCAMMScheduleEditorDlg::OnSelchangeSchDataStarttime() 
{
	// TODO: Add your control notification handler code here
	int nCursel = m_CbStartTime.GetCurSel();
	DWORD dwTimeInter = GetStartEndTimeInterval();
	int nIndex = dwTimeInter/30;

	xUpdateData(TRUE);

	xUpdateStopTimeCb(m_CbStartTime.GetCurSel());

	int nStopIndex = 0;
	if(m_CbStopTime.IsWindowEnabled())
	{
		nStopIndex = max(0,nIndex);
	}
	else
		nStopIndex = 0;
	m_CbStopTime.SetCurSel(nStopIndex);

//	OnSelchangeAlarmInterval();	
	PostMessage(WM_COMMAND ,MAKEWPARAM(IDC_SCH_DATA_ALARM_INTERVAL,CBN_SELCHANGE),(LPARAM)m_cbAlarmLeadTime.m_hWnd);
	xUpdateData(FALSE);
}

void CCAMMScheduleEditorDlg::OnSelchangeSchDataStoptime() 
{
	
}

void CCAMMScheduleEditorDlg::OnSelchangeNotetype()
{
	Check_NoteType();
}

void CCAMMScheduleEditorDlg::xUpdateStopTimeCb(int startIndex)
{
	m_CbStopTime.ResetContent();
	TCHAR hourStr[50];
	for (int i=startIndex ;i<m_CbStartTime.GetCount();i++) {
		m_CbStartTime.GetLBText(i,hourStr);
		m_CbStopTime.AddString(hourStr);
	}
	CString hour1159Str = LoadStringFromFile(_T("public"),_T("pm1159"));

	hour1159Str = "PM 11:59";
	m_CbStopTime.AddString(hour1159Str);

}

void CCAMMScheduleEditorDlg::SetMonthInfoPtr(CScheduleMonthInfo *pMi)
{
	m_pMonthInfo = pMi;

}

void CCAMMScheduleEditorDlg::OnSchChkAlarm() 
{
	// TODO: Add your control notification handler code here
	xUpdateData(TRUE);
/*
	CEdit* pAlarmPhone;
	pAlarmPhone = (CEdit*) GetDlgItem(IDC_SCH_DATA_ALARM_PHONENO);
	pAlarmPhone->EnableWindow(m_bAlarm);
*/	
	CComboBox* pAlarmInterval;              
	pAlarmInterval = (CComboBox*) GetDlgItem(IDC_SCH_DATA_ALARM_INTERVAL);
//	int nNoteType = m_ArrayofSupportNoteType[m_cbIndex_NoteType].nNodeType;
// #ifdef ANW_9a9u
//     	pAlarmInterval->EnableWindow(m_bAlarm);
// 		m_cbAlarmLeadTime.SetCurSel(m_nInitAlarmLeadTime);
// #else
	//下面的条件应该修改，直接用start time的状态来判断
	//之前的一部分(m_CbStartTime.GetStyle() & WS_VISIBLE) && 
	if(!(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000008) ) //startTime
		//	if(nNoteType == GSM_CAL_BIRTHDAY || nNoteType == GSM_CAL_REMINDER || nNoteType == GSM_CAL_MEMO)
	{
		//周年纪念这种类型，alarm会影响starttime 修改之 kerm 12.31 
		pAlarmInterval->EnableWindow(FALSE);
		//去掉alarm对start time的控制,直接显示
		m_CbStartTime.EnableWindow(true);
		//让alarm来控制head tiem的状态 kerm 08.1.02
		if(m_bAlarm)
		{
			m_cbAlarmLeadTime.EnableWindow();
			m_cbAlarmLeadTime.SetCurSel(m_nInitAlarmLeadTime);
		}
/*		if(!m_bAlarm)
		{
			m_CbStartTime.SetCurSel(18);
			//			m_AlarmLeadTime =0 ;
			m_cbAlarmLeadTime.SetCurSel(0);
			OnSelchangeSchDataStarttime();
		}
		else
		{
			int StartTimeIndex = m_ScheduleInfo.rowIndex; //  xGetTimeIndex(m_ScheduleInfo.hiStartTime);
			m_CbStartTime.SetCurSel(StartTimeIndex);
			
			xUpdateStopTimeCb(StartTimeIndex);
			
			CString strStart,strStop;
			strStart = m_ScheduleInfo.hiRealStartTime.GetAMPMTimeString();
			strStop = m_ScheduleInfo.hiRealStopTime.GetAMPMTimeString();
			
			
			int nStartEndTimeInterval = GetTimeInterval(strStart,strStop) / 30;
			
			m_CbStopTime.SetCurSel(nStartEndTimeInterval);
			
			m_CbStartTime.SetWindowText(strStart);
			m_CbStopTime.SetWindowText(strStop);
		}
		if(!m_CbStopTime.IsWindowEnabled())
		{
			CString strStart;
			m_CbStartTime.GetWindowText(strStart);
			m_CbStopTime.SetWindowText(strStart);
			
		}*/
	
	
	}
	else
	{
		pAlarmInterval->EnableWindow(m_bAlarm);
		//    m_AlarmLeadTime = m_ScheduleInfo.alarmLeadtime;
		m_cbAlarmLeadTime.SetCurSel(m_nInitAlarmLeadTime);
	}

// #endif


	
	//set the status of the checkbox for "Tone".
	if (m_bAlarm) {
		m_ckTone.EnableWindow();
		m_cbAlarmTimeType.EnableWindow();
 		m_edtAlarmTime.EnableWindow();
   	//m_ckTone.SetCheck();
//		m_cbAlarmday.EnableWindow(true);
//		m_cbAlarmTime.EnableWindow(true);
	}else {
		m_ckTone.EnableWindow(false);
    	m_ckTone.SetCheck(false);
		m_cbAlarmTimeType.EnableWindow(false);
 		m_edtAlarmTime.EnableWindow(false);
//		m_cbAlarmday.EnableWindow(false);
//		m_cbAlarmTime.EnableWindow(false);

      //  m_AlarmLeadTime = 1;
    	xUpdateData(FALSE);
	}
	OnSelchangeAlarmInterval();

}

void CCAMMScheduleEditorDlg::OnSchChkRepeat() 
{
	// TODO: Add your control notification handler code here
	xUpdateData(TRUE);

	//m_bRepeat = m_chkRepeat.GetCheck();
	int nNoteType = m_ArrayofSupportNoteType[m_cbIndex_NoteType].nNodeType;
	if (!m_bRepeat) 
	{
		m_cbRepeatType.SetCurSel(0);
	}
//	if(nNoteType != GSM_CAL_BIRTHDAY )
		m_cbRepeatType.EnableWindow(m_bRepeat);
	
	//增加对alarm的限制，只有repeat 开启才有alarm	kerm add for 9a9u 1.02
	CComboBox* pAlarmInterval;              
	pAlarmInterval = (CComboBox*) GetDlgItem(IDC_SCH_DATA_ALARM_INTERVAL);	
	if(m_bRepeat)
	{
		
		//去掉重复对闹铃的影响 kerm 2.15
		//m_chkAlarm.EnableWindow(m_bRepeat);	
		//m_bAlarm = true;

		//pAlarmInterval->EnableWindow(m_bAlarm);
	//	m_cbAlarmLeadTime.SetCurSel(m_nInitAlarmLeadTime);080526libaoliu
	}
	else
	{
			
		//去掉重复对闹铃的影响 kerm 2.15
		//m_chkAlarm.EnableWindow(m_bRepeat);	
		//kerm add for 9a9u 1.15
		//m_chkAlarm.SetCheck(m_bRepeat);
		//m_cbAlarmLeadTime.EnableWindow(m_bRepeat);
		//m_bAlarm = false;
		//pAlarmInterval->EnableWindow(m_bAlarm);
	//	m_cbAlarmLeadTime.SetCurSel(m_nInitAlarmLeadTime);//080526libaoliu
	}
	

//	else
//	{
//		m_cbRepeatType.EnableWindow(FALSE);
//		m_cbRepeatType.SetCurSel(4);
//	}

//	CEdit* pRepeatInterval;
//	pRepeatInterval = (CEdit*) GetDlgItem(IDC_SCH_DATA_REPEAT_INTERVAL);
//	pRepeatInterval->EnableWindow(m_bRepeat);
	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].bDisableEndDateWhenRepeat)
	{
		if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000010) //StopDate
		{
			m_cbStopDate.EnableWindow(!m_bRepeat);
			m_StopDate = m_StartDate;
			xUpdateData(false);
		}
	}
	CComboBox* pWhichWeek;
	pWhichWeek = (CComboBox*) GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHWEEK);
	pWhichWeek->EnableWindow(m_bRepeat);

	CComboBox* pRepeatWeekday;
	pRepeatWeekday = (CComboBox*) GetDlgItem(IDC_SCH_DATA_REPEAT_WEEKDAY);
	pRepeatWeekday->EnableWindow(m_bRepeat);

	CButton* pUntil;
	pUntil = (CButton*) GetDlgItem(IDC_SCH_CHK_REPEAT_Until);
	pUntil->EnableWindow(m_bRepeat);

	m_chkRepeatEndDate.EnableWindow(m_bRepeat);
	m_stRepeatEndData.EnableWindow(m_bRepeat);
	m_spRepeatinterval.EnableWindow(m_bRepeat);
	m_edRepeatInterval.EnableWindow(m_bRepeat);
	m_scRepeatinterval.EnableWindow(m_bRepeat);
	m_scRepeatON.EnableWindow(m_bRepeat);

	CEdit* pWhichDate;
	pWhichDate = (CEdit*) GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE);
	if(m_bRepeat)
	{
		BOOL bSupport = false;
		if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000400) //repeatEndDate
			bSupport = true;
		if(bSupport == false)
		{
			pWhichDate->EnableWindow(false);
			m_chkRepeatEndDate.EnableWindow(false);
			m_stRepeatEndData.EnableWindow(false);
		}
		else
		{
			if((m_chkRepeatEndDate.GetStyle() & WS_VISIBLE))
				pWhichDate->EnableWindow(m_chkRepeatEndDate.GetCheck());
			else
				pWhichDate->EnableWindow(true);
		}

		if(m_cbRepeatType.GetItemData(m_cbRepeatType.GetCurSel()) ==YEARLY && m_ArrayofSupportNoteType[m_cbIndex_NoteType].nRepeatIntervalYear ==0)
		{
			m_spRepeatinterval.EnableWindow(false);
			m_edRepeatInterval.EnableWindow(false);
			m_edRepeatInterval.SetWindowText(_T("1"));
		}

	}
	else
		pWhichDate->EnableWindow(false);

	UpdateDateofWeekCtlStates();

//	m_wndUntilDate.EnableButtonWindow(m_bRepeatUntil && m_bRepeat);
}

void CCAMMScheduleEditorDlg::OnSCHCHKREPEATUntil() 
{
	// TODO: Add your control notification handler code here
	xUpdateData(TRUE);

	CEdit* pWhichDate;
	pWhichDate = (CEdit*) GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE);
	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000400)
		pWhichDate->EnableWindow(m_chkRepeatEndDate.GetCheck() && m_bRepeat);
	else
		pWhichDate->EnableWindow(false);

//	m_wndUntilDate.EnableButtonWindow(m_bRepeatUntil && m_bRepeat);
	
}


void CCAMMScheduleEditorDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	OnSchChkAlarm();
//	OnSchChkRepeat();
	OnSCHCHKREPEATUntil();
	OnSelchangeSchDataRepeatType();

	CEdit* pEdit;
	long oldStyle;
	//	pEdit = (CEdit*)GetDlgItem(IDC_SCH_DATA_ALARM_PHONENO);
	//	oldStyle = GetWindowLong(pEdit->GetSafeHwnd(),GWL_STYLE);
	//	SetWindowLong(pEdit->GetSafeHwnd(),GWL_STYLE,oldStyle | ES_NUMBER);
	//	pEdit = NULL;

	pEdit = (CEdit*)GetDlgItem(IDC_SCH_DATA_REPEAT_INTERVAL);
	oldStyle = GetWindowLong(pEdit->GetSafeHwnd(),GWL_STYLE);
	SetWindowLong(pEdit->GetSafeHwnd(),GWL_STYLE,oldStyle | ES_NUMBER);
	pEdit = NULL;


	///directly show the value of hour-time, not by the item of combo. 
/*	CString csStartTim,csStopTime;
	csStartTim=(LPCTSTR)m_ScheduleInfo.hiRealStartTime.GetAMPMTimeString();
	csStopTime=(LPCTSTR)m_ScheduleInfo.hiRealStopTime.GetAMPMTimeString();
	m_CbStartTime.SetWindowText(csStartTim);
	m_CbStopTime.SetWindowText(csStopTime);*/
    ///
}

//当重复组合框发生变化，会调用下面的函数
void CCAMMScheduleEditorDlg::OnSelchangeSchDataRepeatType() 
{
	BOOL bMonthWeek = FALSE;
	xUpdateData(TRUE);
	//
	UpdateDateofWeekCtlStates();
/*
	if (m_RepeatType == WEEKLYMONTHLY) {
		bMonthWeek = TRUE;
	}
	CComboBox* pWhichWeek;
	pWhichWeek = (CComboBox*) GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHWEEK);
	pWhichWeek->ShowWindow(bMonthWeek);

	CComboBox* pWhichDay;
	pWhichDay = (CComboBox*) GetDlgItem(IDC_SCH_DATA_REPEAT_WEEKDAY);
	pWhichDay->ShowWindow(bMonthWeek);

	TCHAR RepeatType[50];
	m_cbRepeatType.GetLBText(m_RepeatType,RepeatType);
	SetDlgItemText(IDC_SCH_LT_REPEAT_INTERVALUNIT,RepeatType);*/

	//下面暂时用不到
	if(m_scRepeatinterval.GetStyle() & WS_VISIBLE)
	{
		int nType = m_cbRepeatType.GetItemData(m_cbRepeatType.GetCurSel());
		TCHAR szKey[MAX_PATH];
		TCHAR szText[MAX_PATH];
		switch(nType)
		{
		case DAILY:
			wsprintf(szKey,_T("IDS_REPEATINTERVAL_DAYS"));
			break;
		case WEEKLY:
		case TWOWEEKLY:
		case WEEKDAYS:
		case WEEKEND:
			wsprintf(szKey,_T("IDS_REPEATINTERVAL_WEEKS"));
			break;
		case MONTHLY:
		case WEEKLYMONTHLY:
			wsprintf(szKey,_T("IDS_REPEATINTERVAL_MOONTHS"));
			break;
		case YEARLY:
			wsprintf(szKey,_T("IDS_REPEATINTERVAL_YEARS"));
			break;
		}
		al_GetSettingString(_T("public"),szKey,theApp.m_szRes,szText);
		m_scRepeatinterval.SetWindowText(szText);
		if(nType ==YEARLY && m_ArrayofSupportNoteType[m_cbIndex_NoteType].nRepeatIntervalYear ==0)
		{
			m_spRepeatinterval.EnableWindow(false);
			m_edRepeatInterval.EnableWindow(false);
			m_edRepeatInterval.SetWindowText(_T("1"));
		}
		else
		{
			m_spRepeatinterval.EnableWindow(true);
			m_edRepeatInterval.EnableWindow(true);
		}
	}

}

ScheduleDate CCAMMScheduleEditorDlg::xGetScheduleDate(COleDateTimeEx utilWhichDate)
{
	ScheduleDate sd;
	sd.SetDate(utilWhichDate.GetYear(),utilWhichDate.GetMonth(),utilWhichDate.GetDay());

	return sd;
}






BOOL CCAMMScheduleEditorDlg::xIsDataModified()
{

	if (m_ScheduleInfo.dtStartDate != m_StartDate)
	{	return TRUE;}
	int nNoteType = m_ArrayofSupportNoteType[m_cbIndex_NoteType].nNodeType;
	if(m_cbNoteType.GetStyle()& WS_VISIBLE)
	{
		if (nNoteType != m_ScheduleInfo.uiNodeType)		//类型改变
		{	
		return true;}
	}


	TCHAR txt[MAX_LENGTH];
	CScheduleTimeInfo stiStartTime;
	CScheduleTimeInfo stiStopTime;
	memset(txt,0x00,MAX_LENGTH);
	if((m_CbStartTime.GetStyle() & WS_VISIBLE) && m_CbStartTime.IsWindowEnabled())
	{
		m_CbStartTime.GetWindowText(txt,MAX_LENGTH);
		CString txtStr=txt;
		stiStartTime.ParseAMPMTimeString(txtStr);
		if (m_ScheduleInfo.hiRealStartTime != stiStartTime)		//开始时间改变
		{
			return TRUE;
		}
	}

	if((m_cbStopDate.GetStyle() & WS_VISIBLE) && m_cbStopDate.IsWindowEnabled())
	{
		if(m_ScheduleInfo.dtStopDate.GetYear() != m_StopDate.GetYear() ||		
			m_ScheduleInfo.dtStopDate.GetMonth() != m_StopDate.GetMonth() ||
			m_ScheduleInfo.dtStopDate.GetDay() != m_StopDate.GetDay() )
		{
		return TRUE;}							//结束时间改变
	/*	if (m_ScheduleInfo.dtStopDate != m_StopDate) 
		{
			return TRUE;
		}*/
	}
	memset(txt,0x00,MAX_LENGTH);
	if((m_CbStopTime.GetStyle() & WS_VISIBLE) && m_CbStopTime.IsWindowEnabled())
	{
		m_CbStopTime.GetWindowText(txt,MAX_LENGTH);
		CString  txtStr=txt;
		stiStopTime.ParseAMPMTimeString(txtStr);
		if (m_ScheduleInfo.hiRealStopTime != stiStopTime) 
		{
			return TRUE;
		}
	}
	if((m_cbStopTimeType.GetStyle() & WS_VISIBLE)  && (m_edtStopTime.GetStyle() & WS_VISIBLE))
	{
		  COleDateTime dtStop;
		  if(GetStopDateTime(dtStop))
		  {
				int nHour=0, nMin=0;
				m_ScheduleInfo.hiRealStopTime.Get24HourTime(nHour, nMin);
			//	COleDateTime dtOriStopTime;
			//	dtOriStopTime.SetDateTime(m_ScheduleInfo.dtStopDate.GetYear(),m_ScheduleInfo.dtStopDate.GetMonth(),m_ScheduleInfo.dtStopDate.GetDay(),nHour, nMin,0);
				if (m_ScheduleInfo.dtStopDate.GetYear() != dtStop.GetYear() ||
					m_ScheduleInfo.dtStopDate.GetMonth() != dtStop.GetMonth() ||
					m_ScheduleInfo.dtStopDate.GetDay() != dtStop.GetDay() ||
					nHour != dtStop.GetHour() ||
					nMin != dtStop.GetMinute() ) 
				{
				return TRUE;}
		  }
	}


	//	if (m_ScheduleInfo.rowIndex != m_StartTime) {
	//		return TRUE;
	//	}
	
	//	if (m_ScheduleInfo.n30MinBlockCnt != (m_StopTime + (m_CbStartTime.GetCount() - m_CbStopTime.GetCount()) - m_StartTime + 1)) {
	//		return TRUE;
	//	}
	if((GetDlgItem(IDC_SCH_DATA_TITLE)->GetStyle() & WS_VISIBLE))
	{
		if (m_ScheduleInfo.csTitle.GetLength() != m_Title.GetLength())
			return TRUE;
		if (m_Title.GetLength() != 0) {
			if (m_ScheduleInfo.csTitle != m_Title)
			{	
				return TRUE;
			}
		}
	}
/*	if((GetDlgItem(IDC_SCH_DATA_MEMO)->GetStyle() & WS_VISIBLE))
	{
		if (m_ScheduleInfo.csMemo.GetLength() != m_Memo.GetLength())
			return TRUE;
		if (m_Memo.GetLength() != 0) {
			if (m_ScheduleInfo.csMemo != m_Memo) {
				return TRUE;
			}
		}
	}*/
	if((m_edDescription.GetStyle() & WS_VISIBLE))
	{
		CString str;
		m_edDescription.GetWindowText(str);
		if (m_ScheduleInfo.csMemo.GetLength() != str.GetLength())
			return TRUE;
		if (str.GetLength() != 0) {
			if (str.CompareNoCase(m_ScheduleInfo.csMemo) !=0) 
			{
				return TRUE;
			}
		}
	}

	//Alarm
	if ((m_chkAlarm.GetStyle() & WS_VISIBLE) && m_ScheduleInfo.haveAlarm != m_bAlarm) 
	{	return TRUE;}
	if ((m_chkAlarm.GetStyle() & WS_VISIBLE) && m_bAlarm) 	
	{
		if((m_ckTone.GetStyle() & WS_VISIBLE))
		{
			if (m_ScheduleInfo.bhaveTone != m_bTone) 
				return TRUE;
		}
		COleDateTime dtAlarm;
  		int nHour=0, nMin=0;
		m_ScheduleInfo.hiRealStartTime.Get24HourTime(nHour, nMin);

		COleDateTime dtStartDate24Hur;
		//定义一个datetime用当前的scheduleinfo的年月日时分秒（为零）来初始化
		dtStartDate24Hur.SetDateTime(m_ScheduleInfo.dtStartDate.GetYear(), m_ScheduleInfo.dtStartDate.GetMonth(), m_ScheduleInfo.dtStartDate.GetDay(),nHour,nMin,0);
		BOOL bTimeOnly = FALSE;
		CString s3;//for test
		if(GetAlarmDateTime(dtStartDate24Hur ,dtAlarm,bTimeOnly) == TRUE)
		{
			if(bTimeOnly)
			{
				if(m_ScheduleInfo.dtAlarmDateTime.GetHour() !=dtAlarm.GetHour() ||
					m_ScheduleInfo.dtAlarmDateTime.GetMinute() !=dtAlarm.GetMinute() ||
					m_ScheduleInfo.dtAlarmDateTime.GetSecond() !=dtAlarm.GetSecond())
					return TRUE;
									

			}
			else
			{
				if(m_ScheduleInfo.dtAlarmDateTime.GetYear() !=dtAlarm.GetYear() ||
					m_ScheduleInfo.dtAlarmDateTime.GetMonth() !=dtAlarm.GetMonth() ||
					m_ScheduleInfo.dtAlarmDateTime.GetDay() !=dtAlarm.GetDay()||
					m_ScheduleInfo.dtAlarmDateTime.GetHour() !=dtAlarm.GetHour() ||
					m_ScheduleInfo.dtAlarmDateTime.GetMinute() !=dtAlarm.GetMinute() ||
					m_ScheduleInfo.dtAlarmDateTime.GetSecond() !=dtAlarm.GetSecond())

					
				{/*	s3.Format(L"%d-- %d, %d---%d, %d---%d, %d---%d, %d---%d, %d---%d",
							m_ScheduleInfo.dtAlarmDateTime.GetYear(), dtAlarm.GetYear(),
							m_ScheduleInfo.dtAlarmDateTime.GetMonth(), dtAlarm.GetMonth(),
							m_ScheduleInfo.dtAlarmDateTime.GetDay(), dtAlarm.GetDay(),
							m_ScheduleInfo.dtAlarmDateTime.GetHour(), dtAlarm.GetHour(),
							m_ScheduleInfo.dtAlarmDateTime.GetMinute(), dtAlarm.GetMinute(),
							m_ScheduleInfo.dtAlarmDateTime.GetSecond(), dtAlarm.GetSecond()
							);
					AfxMessageBox(s3);*/
//				if (m_ScheduleInfo.dtAlarmDateTime.m_dt != dtAlarm.m_dt) 
					return TRUE;}
			}
		}

	/*	if(m_cbAlarmLeadTime.IsWindowVisible() && GetAlarmLeadTime() != 65535)
		{
  			int nHour=0, nMin=0;
			m_ScheduleInfo.hiStartTime.Get24HourTime(nHour, nMin);

			COleDateTime dtStartDate24Hur;
			dtStartDate24Hur.SetDateTime(m_ScheduleInfo.dtStartDate.GetYear(), m_ScheduleInfo.dtStartDate.GetMonth(), m_ScheduleInfo.dtStartDate.GetDay(),nHour,nMin,0);

			COleDateTimeSpan dtAlarmLeadTimeSpan;
			dtAlarmLeadTimeSpan.SetDateTimeSpan(0,0,GetAlarmLeadTime(),0);
			dtAlarm = dtStartDate24Hur - dtAlarmLeadTimeSpan;
		}
		else
		{
			int nY,nM,nD,nH,nMin,ns;
			if(m_cbAlarmday.IsWindowVisible())
			{
				nY = m_Alarmday.GetYear();
				nM = m_Alarmday.GetMonth();
				nD = m_Alarmday.GetDay();
			}
			else
			{
				nY = m_StartDate.GetYear();
				nM = m_StartDate.GetMonth();
				nD = m_StartDate.GetDay();
			}

			if(m_cbAlarmTime.IsWindowVisible())
			{
				nH =m_AlarmTime.GetHour() ;
				nMin =m_AlarmTime.GetMinute(); 
				ns =m_AlarmTime.GetSecond();
			}
			else
			{
				TCHAR txt[MAX_LENGTH];
				memset(txt,0x00,MAX_LENGTH);
				m_CbStartTime.GetWindowText(txt,MAX_LENGTH);

				CString txtStr=txt;
				CScheduleTimeInfo startTimeInfo;
				startTimeInfo.ParseAMPMTimeString(txtStr);

				startTimeInfo.Get24HourTime(nH,nMin);
				ns =0;
			}
			dtAlarm.SetDateTime(nY,nM,nD,nH,nMin, ns);

			if (m_ScheduleInfo.dtAlarmDateTime != dtAlarm) 
				return TRUE;
		}*/
	}

	//Calendar
	//telnumber
	if((m_etAlarmPhoneNo.GetStyle() & WS_VISIBLE))
	{
		if (m_ScheduleInfo.telnumber.GetLength() != m_ExtraBytes.GetLength()+m_AlarmPhoneNo.GetLength())
			return TRUE;
		if (m_AlarmPhoneNo.GetLength() != 0) {
			if (m_ScheduleInfo.telnumber != m_ExtraBytes+m_AlarmPhoneNo) {
				return TRUE;
			}
		}
	}
	//Location
	if((m_etLocation.GetStyle() & WS_VISIBLE))
	{
		CString	_csLocation(m_csLocation);
		if (m_ScheduleInfo.cuLocation.GetLength()!=_csLocation.GetLength()) {
			return TRUE;
		}else if (m_ScheduleInfo.cuLocation.GetLength()!=0
	     		&& (m_ScheduleInfo.cuLocation) != _csLocation) {
			return TRUE;
		}
	}

	//Priority
	if(m_cbPriority.GetStyle() & WS_VISIBLE)
	{
		if(m_ScheduleInfo.uiPrivate != m_cbPriority.GetItemData(m_cbPriority.GetCurSel()))
		{
		return true;}
	}

	//Repeat
	if((m_chkRepeat.GetStyle() & WS_VISIBLE))
	{
		if (m_ScheduleInfo.haveRepeat != m_bRepeat) 
		{return TRUE;}
		if (m_bRepeat)
		{
			CString str;
			int nRepeatType = m_cbRepeatType.GetItemData(m_cbRepeatType.GetCurSel());
			int Oldrepettype = m_ScheduleInfo.repeatType;
			if(Oldrepettype == WEEKLY && m_ScheduleInfo.nDayofWeekMask == 62 )
				Oldrepettype = WEEKDAYS;
			else if(Oldrepettype == WEEKLY && m_ScheduleInfo.nDayofWeekMask == 65 )
				Oldrepettype = WEEKEND;
			if(nRepeatType == WEEKLY && m_ScheduleInfo.nDayofWeekMask == 62 )
				nRepeatType = WEEKDAYS;
			else if(nRepeatType == WEEKLY && m_ScheduleInfo.nDayofWeekMask == 65 )
				nRepeatType = WEEKEND;
			if(m_edRepeatInterval.GetStyle() & WS_VISIBLE)
			{
				m_edRepeatInterval.GetWindowText(str);
		//		str = m_RepeatFrequency;
				if (m_ScheduleInfo.repeatFrequency != _ttoi(str)) {
					return TRUE;
				}
				if(nRepeatType ==WEEKLY && m_ScheduleInfo.repeatFrequency == 2)
					nRepeatType=TWOWEEKLY;
				if(m_ScheduleInfo.repeatType ==WEEKLY && m_ScheduleInfo.repeatFrequency == 2)
					m_ScheduleInfo.repeatType=TWOWEEKLY;
			}
			if (Oldrepettype != nRepeatType) {
				return TRUE;
			}
			if (m_ScheduleInfo.rfWhichDay != m_WhichDay) {
				return TRUE;
			}
		//	if (m_ScheduleInfo.rfWhichWeek != m_RepeatWhichWeek) {
		//		return TRUE;
		//	}
			//Repeat until
			if((m_chkRepeatEndDate.GetStyle() & WS_VISIBLE))
			{

				if (m_ScheduleInfo.haveRepeatUntil != m_chkRepeatEndDate.GetCheck()) {
					return TRUE;
				}
				if (m_chkRepeatEndDate.GetCheck()) {
					if (m_ScheduleInfo.utilWhichDate._yy != m_UntilDate.GetYear()) {
						return TRUE;
					}
					if (m_ScheduleInfo.utilWhichDate._mm != m_UntilDate.GetMonth()) {
						return TRUE;
					}
					if (m_ScheduleInfo.utilWhichDate._dd != m_UntilDate.GetDay()) {
						return TRUE;
					}
				}
			}
			else if(GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE)->GetStyle() & WS_VISIBLE)
			{
				if(GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE)->IsWindowEnabled())
				{
					if (m_ScheduleInfo.haveRepeatUntil != 1 ) 
						return TRUE;
					if (m_ScheduleInfo.utilWhichDate._yy != m_UntilDate.GetYear()) 
						return TRUE;
					if (m_ScheduleInfo.utilWhichDate._mm != m_UntilDate.GetMonth()) 
						return TRUE;
					if (m_ScheduleInfo.utilWhichDate._dd != m_UntilDate.GetDay()) 
						return TRUE;
				}
			}

			//Repeat Dayofweekmask 
			int nDayMask = 0;
			if((m_ScheduleInfo.repeatType == TWOWEEKLY || m_ScheduleInfo.repeatType == WEEKLY) && (m_chWeek1.GetStyle() & WS_VISIBLE ))
			{
				if((m_chWeek1.GetStyle() & WS_VISIBLE ) && m_chWeek1.GetCheck())
						nDayMask |=1;
				if((m_chWeek2.GetStyle() & WS_VISIBLE ) && m_chWeek2.GetCheck())
						nDayMask |=2;
				if((m_chWeek3.GetStyle() & WS_VISIBLE ) && m_chWeek3.GetCheck())
						nDayMask |=4;
				if((m_chWeek4.GetStyle() & WS_VISIBLE ) && m_chWeek4.GetCheck())
						nDayMask |=8;
				if((m_chWeek5.GetStyle() & WS_VISIBLE ) && m_chWeek5.GetCheck())
						nDayMask |=16;
				if((m_chWeek6.GetStyle() & WS_VISIBLE ) && m_chWeek6.GetCheck())
						nDayMask |=32;
				if((m_chWeek7.GetStyle() & WS_VISIBLE ) && m_chWeek7.GetCheck())
						nDayMask |=64;

				if(nDayMask != m_ScheduleInfo.nDayofWeekMask)
					return true;
			}

		}

	}





	return FALSE;
}

void CCAMMScheduleEditorDlg::OnCancel() 
{
	if(m_bViewOnly) 
	{
		CDialog::OnCancel();
		return ;
	}

	// TODO: Add extra cleanup here
	xUpdateData(TRUE);
	if (xIsDataModified()) 
	{
		int CancelFlag =CalendarMessageBox(m_hWnd, _T("MSG_DATA_LOST"),MB_YESNO);
		if (CancelFlag == IDNO)
			return;
	}
	CDialog::OnCancel();

}

BOOL CCAMMScheduleEditorDlg::xCheckStringLegal(CString s) {
	if (s.GetLength() != 1) {
		return FALSE;
	}
	//	CString sNum[]={"0","1","2","3","4","5","6","7","8","9","+","*","#","P","p","X","x"};
	CString sNum[]={"0","1","2","3","4","5","6","7","8","9","+","*","#","p","w"};
	for (int i=0;i<15;i++) {
		if (s == sNum[i]) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CCAMMScheduleEditorDlg::xCheckPhoneLegal(CString s) {
	for (int i=0;i<s.GetLength();i++) {
		if (!xCheckStringLegal(s.Mid(i,1))) {
			return FALSE;
		}
	}
	return TRUE;
}

void CCAMMScheduleEditorDlg::OnUpdateSchDataAlarmPhoneno() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
//	AfxMessageBox(m_AlarmPhoneNo);
/*	CString s;
	CString newS = "";
	m_etAlarmPhoneNo.GetWindowText(s);
	for (int i=0;i<s.GetLength();i++) {
		if (xCheckNumber(s.Mid(i,1))) {
			newS += s.Mid(i,1);
		}
	}
	m_AlarmPhoneNo = newS;
	UpdateData(FALSE);

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_SCH_DATA_ALARM_PHONENO);
	SetWindowLong(SetWindowLong,GWL_STYLE,ES_NUMBER);
	CString strText;
	pEdit->GetWindowText(strText);
	int nIndex = newS.GetLength();
	strText = strText.Mid(nIndex, 1);

	// Get the text extent of the character.
	CDC* pDC = pEdit->GetDC();
	CSize sz = pDC->GetTextExtent(strText);
	pEdit->ReleaseDC(pDC);

	CPoint pt = pEdit->PosFromChar(nIndex);
	m_etAlarmPhoneNo.SetCaretPos(pt);
*/
//	m_etAlarmPhoneNo.SetSel(HIWORD(newS.GetLength()+1), TRUE);
//	AfxMessageBox(s);
//	AfxMessageBox("UPdate");
	
}


HBRUSH CCAMMScheduleEditorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
//	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
//	pDC->SetBkColor(RGB(235, 243, 246));
//	return m_hBrush;

	if(nCtlColor== CTLCOLOR_DLG)	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(0, 0, 0));
		return m_hBrush;
	}
	else
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CCAMMScheduleEditorDlg::ShowCtrlItem(bool _bShowCtrlItem) 
{
	m_ctrlOk.ShowWindow(_bShowCtrlItem);
	m_ctrlCancel.ShowWindow(_bShowCtrlItem);
}
//根据不同的类型控制各控件的激活情况
void CCAMMScheduleEditorDlg::Check_NoteType() 
{
	xUpdateData(TRUE);
	
	if (m_cbIndex_NoteType >= m_ArrayofSupportNoteType.GetSize()) 
		return;
	if (-1 == m_cbIndex_NoteType)
			return;
	m_nMaxAlarmLength = m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax;
	m_nMaxStopLength= m_ArrayofSupportNoteType[m_cbIndex_NoteType].nEndTimeIntervalMax;
	//下面两个函数只是插入内容，没有对控件的显示状态进行判断
	InsertAlarmLeadTypetoComboCtrl(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwAlarmLeadTimeType);
	InsertRepeatTypetoComboCtrl(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwRepeatType);
	//结束时间类型，没有用到
	if((m_cbStopTimeType.GetStyle() & WS_VISIBLE))
		InsertEndTimeIntervalTypetoComboCtrl(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwEndTimeIntervalType);
	//闹铃时间类型，没有用到 9@9u使用alarmleadtype
	if((m_cbAlarmTimeType.GetStyle() & WS_VISIBLE))
		InsertAlarmTimeIntervalTypetoComboCtrl(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwAlarmTimeIntervalType);
	//设定闹钟选中状态
	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].nDefaultAlarmcheck >=0)
	{
		m_chkAlarm.SetCheck(m_ArrayofSupportNoteType[m_cbIndex_NoteType].nDefaultAlarmcheck);
		//增加下面代码，处理切换类型时alarm combox更新不正确的问题 kerm add for 9a9u 1.16
		m_bAlarm = m_ArrayofSupportNoteType[m_cbIndex_NoteType].nDefaultAlarmcheck;
		//修改条件，根据ini中的AlarmCheckdefault来设置alarm的状态，原来为true kerm add for 9a9u 08.1.02
		//m_chkAlarm.EnableWindow(m_ArrayofSupportNoteType[m_cbIndex_NoteType].nDefaultAlarmcheck);
		//kerm change 2.15
		m_chkAlarm.EnableWindow(true);
		
	}
	else
	{
		m_chkAlarm.SetCheck(m_ScheduleInfo.haveAlarm);
		m_bAlarm = m_ScheduleInfo.haveAlarm;
		m_chkAlarm.EnableWindow(false);
	}
	TCHAR szText[MAX_PATH];
//	int nNoteType = m_ArrayofSupportNoteType[m_cbIndex_NoteType].nNodeType;
	int nRepeat = m_ScheduleInfo.haveRepeat ;
	int RepeatType = m_ScheduleInfo.repeatType;
	int nDayofWeekMask = m_ScheduleInfo.nDayofWeekMask;
	//下面没用到
	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000002) //text2
	{
		if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].bPhoneNoText2)
		{
 			m_etAlarmPhoneNo.ShowWindow(true);
     		CEdit* pAlarmPhone;
    		pAlarmPhone = (CEdit*) GetDlgItem(IDC_SCH_DATA_ALARM_PHONENO);
			pAlarmPhone->EnableWindow(TRUE);
			m_etLocation.ShowWindow(false);

		}
		else
		{
 			m_etAlarmPhoneNo.ShowWindow(false);
    		CEdit* pAlarmPhone;
    		pAlarmPhone = (CEdit*) GetDlgItem(IDC_SCH_DATA_ALARM_PHONENO);
    		pAlarmPhone->EnableWindow(FALSE);
			//m_etLocation.ShowWindow(true);
			m_etLocation.ShowWindow(SW_HIDE);//080612libaoliu

			m_etLocation.EnableWindow(TRUE);
		}
		m_stAlarmPhoneNo.EnableWindow(TRUE);
	}
	else
	{
	/*	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].bPhoneNoText2)
		{
 			m_etAlarmPhoneNo.ShowWindow(true);
			m_etLocation.ShowWindow(false);
		}
		else
		{
			m_etAlarmPhoneNo.ShowWindow(false);
			m_etLocation.ShowWindow(true);
		}*/
		m_etAlarmPhoneNo.EnableWindow(FALSE);
		m_etLocation.EnableWindow(FALSE);
		m_etLocation.ShowWindow(SW_HIDE);//080612libaoliu
		m_stAlarmPhoneNo.EnableWindow(FALSE);
	}
    al_GetSettingString(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText2ID, theApp.m_szRes ,szText);
	m_stAlarmPhoneNo.SetWindowText(szText);

    al_GetSettingString(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText1ID, theApp.m_szRes ,szText);
	m_stLtTitle.SetWindowText(szText);
  
	al_GetSettingString(_T("public"),m_ArrayofSupportNoteType[m_cbIndex_NoteType].stText3ID, theApp.m_szRes ,szText);
	m_stDescription.SetWindowText(szText);

	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000004) //StratDate
		m_cbStartDate.EnableWindow(true);
	else
		m_cbStartDate.EnableWindow(false);

	//居然通过start time来区别周年纪念，
	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000008) //startTime
		m_CbStartTime.EnableWindow(true);
	else	//在这里将周年纪念的开始时间禁用了
	{	m_CbStartTime.EnableWindow(false); }

	if((m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000004 ) ||
		(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000008 ))
		m_stStartTime.EnableWindow(true);
	else
		m_stStartTime.EnableWindow(false);

	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000010) //StopDate
	{
		m_cbStopDate.EnableWindow(true);
		//stopdate == startdate   kerm change 2.19
		//m_StopDate  = m_ScheduleInfo.dtStopDate ;
		m_StopDate  = m_ScheduleInfo.dtStartDate;
	}
	else
		m_cbStopDate.EnableWindow(false);

	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000020) //StopTime
		m_CbStopTime.EnableWindow(true);
	else
		m_CbStopTime.EnableWindow(false);

	if((m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000010 ) ||
		(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000020 ))
		m_stStopTime.EnableWindow(true);
	else
		m_stStopTime.EnableWindow(false);

	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000080) //alarmTime
	{
		if(m_bAlarm && GetAlarmLeadTime() == 65535)
		{
			if(IsEnabledAlarmTime())
				m_cbAlarmTime.EnableWindow(true);
			else 
				m_cbAlarmTime.EnableWindow(false);
		}
		else
			m_cbAlarmTime.EnableWindow(false);
	}
	else
		m_cbAlarmTime.EnableWindow(false);

	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000100) //alarmTone
		m_ckTone.EnableWindow(true);
	else
		m_ckTone.EnableWindow(false);

	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000200) //repeat
	{
		
		m_chkRepeat.EnableWindow(true);
		m_cbRepeatType.EnableWindow(true);
		//这个地方取ini中的nRepeatdefaulType，如果不存在默认返回0，
		//例如 周年纪念 存在默认 直接disable repeattype
		if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].nRepeatdefaulType)
		{
			
			m_chkRepeat.SetCheck();
//			m_cbRepeatType.SetCurSel(m_ArrayofSupportNoteType[m_cbIndex_NoteType].nRepeatdefaulType);
			SetRepeatType(m_ArrayofSupportNoteType[m_cbIndex_NoteType].nRepeatdefaulType,nDayofWeekMask);
			//增加下面两句，kerm add for 9a9u 08.1.02
			//周年纪念存在默认，但是需要把alram enable了，如果不这样做，将无法操作alarm
			//因为9a9u的alarm与repeat 是相关联的
			m_chkAlarm.EnableWindow(true);
			m_cbRepeatType.EnableWindow(true);

			m_chkRepeat.EnableWindow(false);
			m_cbRepeatType.EnableWindow(false);
			//下面没有使用
			if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].nRepeatdefaulType ==YEARLY && m_ArrayofSupportNoteType[m_cbIndex_NoteType].nRepeatIntervalYear ==0)
			{
				m_spRepeatinterval.EnableWindow(false);
				m_edRepeatInterval.EnableWindow(false);
				m_edRepeatInterval.SetWindowText(_T("1"));
			}
			else
			{
				m_spRepeatinterval.EnableWindow(true);
				m_edRepeatInterval.EnableWindow(true);
			}

		}
		else	//当不存在默认时，
		{
			m_chkRepeat.EnableWindow(true);
			if(nRepeat)
			{
				
				m_chkRepeat.SetCheck();
//				m_cbRepeatType.SetCurSel(RepeatType);
				
				SetRepeatType(RepeatType,nDayofWeekMask);
				m_cbRepeatType.EnableWindow(true);

				if(RepeatType ==YEARLY && m_ArrayofSupportNoteType[m_cbIndex_NoteType].nRepeatIntervalYear ==0)
				{
					m_spRepeatinterval.EnableWindow(false);
					m_edRepeatInterval.EnableWindow(false);
					m_edRepeatInterval.SetWindowText(_T("1"));
				}
				else
				{
					m_spRepeatinterval.EnableWindow(true);
					m_edRepeatInterval.EnableWindow(true);
				}
			}
			else
			{
				m_chkRepeat.SetCheck(FALSE);
				m_cbRepeatType.SetCurSel(0);
				m_cbRepeatType.EnableWindow(false);
			}
		}
	}
	else
	{
		m_chkRepeat.SetCheck(false);
		m_chkRepeat.EnableWindow(false);
		m_cbRepeatType.SetCurSel(0);
		m_cbRepeatType.EnableWindow(false);
	}
 	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000400) //repeatEndDate
	{
		if(m_chkRepeat.GetCheck())
		{
			m_chkRepeatEndDate.EnableWindow(true);;
			m_stRepeatEndData.EnableWindow(true);;
			if((m_chkRepeatEndDate.GetStyle() & WS_VISIBLE))
				GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE)->EnableWindow(m_chkRepeatEndDate.GetCheck());
			else
				GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE)->EnableWindow(true);
		}
		else
		{
			m_chkRepeatEndDate.EnableWindow(false);;
			m_stRepeatEndData.EnableWindow(false);;
			GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE)->EnableWindow(false);;
		}
	}
	else
	{
		m_chkRepeatEndDate.EnableWindow(false);;
		m_stRepeatEndData.EnableWindow(false);;
		GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE)->EnableWindow(false);;
	}
	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000800) //alarmLeadTime
	{
		if(m_bAlarm)
		{
			m_cbAlarmLeadTime.EnableWindow(true);
//			m_AlarmLeadTime = m_ScheduleInfo.alarmLeadtime; 
			m_cbAlarmLeadTime.SetCurSel(m_nInitAlarmLeadTime);
		}
		else
		{
			m_cbAlarmLeadTime.SetCurSel(0);
			m_cbAlarmLeadTime.EnableWindow(false);
		}
	}
	else
	{
		m_cbAlarmLeadTime.SetCurSel(0);
		m_cbAlarmLeadTime.EnableWindow(false);
	}

	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00001000) //text3
	{
		m_edDescription.EnableWindow(true);
		m_stDescription.EnableWindow(true);
	}
	else
	{
		m_edDescription.EnableWindow(false);
		m_stDescription.EnableWindow(false);
	}
	UpdateDateofWeekCtlStates();

	//下面是选择周年纪念时的初始化部分，这个if条件比较奇怪...
	//在此之前应该是有动作将starttime给disable了，所以才可以这么用	kerm 08.1.02
	if(!m_CbStartTime.IsWindowEnabled())
	{
		//没有办法，先把start time给开启了
		m_CbStartTime.EnableWindow(true);
		//把闹铃enable了，但是不选中
		m_chkAlarm.EnableWindow();
		m_chkAlarm.SetCheck(false);
//		m_CbStartTime.SetCurSel(18);//Am 9:00
		//修改下面的条件，让start time不受alarm的限制，而是让alarm控制alarm head time
		//不用根据headtime来调整开始时间了
/*		if(m_CbStartTime.IsWindowEnabled())	//修改之前的条件：m_bAlarm
		{
			CString str,strStart;
			int nH = m_ScheduleInfo.dtAlarmDateTime.GetHour();
			int nM = m_ScheduleInfo.dtAlarmDateTime.GetMinute();
			if(nH >=12 )
			{
				str =LoadStringFromFile(_T("public"),_T("PM"));
				if(nH >12)	nH -=12;
			}
			else 
			{
				str =LoadStringFromFile(_T("public"),_T("AM"));
				if(nH == 0) nH =12;
			}
			strStart.Format(_T("%s %02d:%02d"),str,nH,nM);


			m_CbStartTime.SetWindowText(strStart);
		}
		else
			m_CbStartTime.SetCurSel(18);//Am 9:00*/
		//下面的代码是用alarm来控制head time
		if(m_bAlarm)
		{
			m_cbAlarmLeadTime.EnableWindow(true);
		}
		else
			m_cbAlarmLeadTime.EnableWindow(false);
	}
//	if(m_CbStartTime.IsWindowVisible() && m_CbStartTime.IsWindowEnabled() == FALSE && m_bAlarm )
	if((m_CbStartTime.GetStyle() & WS_VISIBLE) && m_CbStartTime.IsWindowEnabled() == FALSE && m_bAlarm &&
//		((m_cbAlarmLeadTime.GetStyle() & WS_VISIBLE)||(m_cbAlarmTime.GetStyle() & WS_VISIBLE)))
		(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000080))
	{
		m_CbStartTime.EnableWindow(true);
		m_cbAlarmLeadTime.SetCurSel(0);
	//	m_AlarmLeadTime = 0;
		m_cbAlarmTime.EnableWindow(false);
		m_cbAlarmLeadTime.EnableWindow(false);
	}
	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000040) //alarmDate
	{
		if(m_bAlarm && GetAlarmLeadTime() == 65535)
			m_cbAlarmday.EnableWindow(true);
		else
			m_cbAlarmday.EnableWindow(false);
	}
	else
		m_cbAlarmday.EnableWindow(false);

	
	if(!m_cbStopDate.IsWindowEnabled())
		m_StopDate = m_StartDate;
	if(m_StopDate < m_StartDate)
		m_StopDate = m_StartDate;

	xUpdateData(FALSE);
//	OnSchChkRepeat();
	OnSelchangeSchDataStarttime();
	if(!m_CbStopTime.IsWindowEnabled())
	{
		CString strStart;
		m_CbStartTime.GetWindowText(strStart);
		m_CbStopTime.SetWindowText(strStart);

	}
	else
	{
		int StartTimeIndex = m_ScheduleInfo.rowIndex; //  xGetTimeIndex(m_ScheduleInfo.hiStartTime);
		m_CbStartTime.SetCurSel(StartTimeIndex);

//		CString strStart,strStop;
//		strStart = m_ScheduleInfo.hiRealStartTime.GetAMPMTimeString();
//		strStop = m_ScheduleInfo.hiRealStopTime.GetAMPMTimeString();

//		m_CbStartTime.SetWindowText(strStart);
//		UpdateStopTime(GetTimeInterval(strStart,strStop),StartTimeIndex);

		xUpdateStopTimeCb(StartTimeIndex);

		CString strStart,strStop;
		strStart = m_ScheduleInfo.hiRealStartTime.GetAMPMTimeString();
		strStop = m_ScheduleInfo.hiRealStopTime.GetAMPMTimeString();

		int nStartEndTimeInterval = GetTimeInterval(strStart,strStop) / 30;

		m_CbStopTime.SetCurSel(nStartEndTimeInterval);

		m_CbStartTime.SetWindowText(strStart);
		m_CbStopTime.SetWindowText(strStop);

	}

}

void CCAMMScheduleEditorDlg::OnSelchangeAlarmInterval()
{
	//si.alarmLeadtime = m_AlarmLeadTime;

	//void SetDateTimeSpan( long lDays, int nHours, int nMins, int nSecs );

	xUpdateData(true);


	COleDateTimeSpan dtAlarmLeadTimeSpan;
//	COleDateTimeEx   dtAlarmDateTime;

	CScheduleInfo si;
//	CString cuAlarmLeadtime = si.getAlarmLeadtime(m_AlarmLeadTime);
  //  CString csAlarmLeadtime = (LPCTSTR)cuAlarmLeadtime;
	if ((m_cbAlarmLeadTime.GetStyle() & WS_VISIBLE) && GetAlarmLeadTime() != 65535)
	{
		m_cbAlarmday.EnableWindow(FALSE);
		m_cbAlarmTime.EnableWindow(FALSE);
    	//calculate span time
        dtAlarmLeadTimeSpan.SetDateTimeSpan(0,0,GetAlarmLeadTime(),0);

    	//get the real alarmleadtime
 //   	dtAlarmDateTime = m_ScheduleInfo.dtStartDate - dtAlarmLeadTimeSpan;
		COleDateTime dtStartTime;
		int nHour=0, nMin=0;
		TCHAR txt[MAX_LENGTH];
		memset(txt,0x00,MAX_LENGTH);
		m_CbStartTime.GetWindowText(txt,MAX_LENGTH);
		CString txtStr=txt;
		CScheduleTimeInfo startTimeInfo;
		startTimeInfo.ParseAMPMTimeString(txtStr);

		startTimeInfo.Get24HourTime(nHour, nMin);

		dtStartTime.SetDateTime(m_StartDate.GetYear(), m_StartDate.GetMonth(), m_StartDate.GetDay(),nHour,nMin,0);
//    	m_Alarmday = m_ScheduleInfo.dtStartDate - dtAlarmLeadTimeSpan;
	 	m_Alarmday = dtStartTime - dtAlarmLeadTimeSpan;
		m_AlarmTime.SetTime(m_Alarmday.GetHour(), m_Alarmday.GetMinute(), m_Alarmday.GetSecond());
	}
	else
	{
		if(!m_cbAlarmday.IsWindowEnabled())
		{
			m_Alarmday.SetDateTime(m_StartDate.GetYear(), m_StartDate.GetMonth(), m_StartDate.GetDay(),0,0,0);
		}
		if(!m_cbAlarmTime.IsWindowEnabled())
		{
			int nHour=0, nMin=0;
			TCHAR txt[MAX_LENGTH];
			memset(txt,0x00,MAX_LENGTH);
			m_CbStartTime.GetWindowText(txt,MAX_LENGTH);
			CString txtStr=txt;
			CScheduleTimeInfo startTimeInfo;
			startTimeInfo.ParseAMPMTimeString(txtStr);
			startTimeInfo.Get24HourTime(nHour, nMin);
			m_AlarmTime.SetTime(nHour, nMin,0);
		}
		if(m_bAlarm )
		{
			if((m_CbStartTime.GetStyle() & WS_VISIBLE) && !(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000008)) //startTime
			{
				m_cbAlarmTime.EnableWindow(FALSE);
				m_cbAlarmLeadTime.EnableWindow(FALSE);
			}
			else
			{
				if(IsEnabledAlarmTime())
				{
					m_cbAlarmTime.EnableWindow(TRUE);
					m_cbAlarmLeadTime.EnableWindow(TRUE);
				}
				else
				{
					m_cbAlarmTime.EnableWindow(FALSE);
					m_cbAlarmLeadTime.EnableWindow(FALSE);
				}
			}

			if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000040) //alarmDate
				m_cbAlarmday.EnableWindow(TRUE);
			else
				m_cbAlarmday.EnableWindow(FALSE);
		}
		else
		{
			m_cbAlarmday.EnableWindow(FALSE);
			m_cbAlarmTime.EnableWindow(FALSE);
			m_cbAlarmLeadTime.EnableWindow(FALSE);
		}
	}

	xUpdateData(false);
}

bool CCAMMScheduleEditorDlg::GetCalendarSupport()
{
//CALLING FUNCTION:
//   bool CCAMMSync::GetCalendarSupport(int& nSupportNum, CArray<bool,bool>& ArraySupportType, int nArrayCount)
	CCAMMSync sc;
	//dlg 调用sync的getsupport 函数 返回 数量 与 类型   注意对type的初始化
	//注意 下面的数组是notetypesupportinfo型的 根sync中的不一样
	if (sc.GetCalendarSupport(m_SizeofSupportNoteType, m_ArrayofSupportNoteType)){
		return true;
	}else {
		return false;
	}
}


void CCAMMScheduleEditorDlg::InsertNoteTypetoComboCtrl()
{//typedef enum {REMIND=1, CALL, MEETING, BIRTHDAY, MEMO} NOTETYPE;//Calendar
	TCHAR csNoteType[MAX_PATH];//nono,2004_1030
	for (int i=0; i<m_SizeofSupportNoteType; i++) 
	{
		al_GetSettingString(_T("public"),m_ArrayofSupportNoteType[i].stTypeID, theApp.m_szRes ,csNoteType);
		//向组合框的列表中增加文字
    	int nIndex = m_cbNoteType.AddString(csNoteType);
		//下面代码 将列表的序号与支持类型的地址对应 在哪里用到？？
		m_cbNoteType.SetItemData(nIndex,(unsigned long)&m_ArrayofSupportNoteType[i]);
	}
}

void CCAMMScheduleEditorDlg::xInitDialog()
{
//	char szProfile[MAX_PATH];
    CRect rect, rect2, rect3;
    COLORREF color;
	BOOL bVisible;
	TCHAR szText[MAX_PATH];

	 TCHAR szFontName[MAX_PATH];
	 int   nFontSize;
	//Load global font setting
	//m_szFont = skin\font.ini
	//读取字体 
	if(!al_GetSettingString(_T("Font"),_T("facename"),theApp.m_szFont,szFontName))
		_tcscpy(szFontName, _T("Arial"));
	//读取字体大小
	if(!al_GetSettingInt(_T("Font"),_T("fontsize"),theApp.m_szFont,nFontSize))
		nFontSize=12;


//	wsprintf(szProfile, "%s%s", theApp.m_szSkin, "Calendar\\AddDlg.ini");
	//根据手机的名称合成其皮肤的路径 格式： ..skin\default\calendar\addDlg_手机名字.ini
	//m_szMobileName_ori 从mobile setting.ini中读取name key
	wsprintf(m_szProfile, _T("%s%sAddDlg_%s.ini"), theApp.m_szSkin, _T("Calendar\\"),theApp.m_szMobileName_ori);
	//如果上述路径不存在 根据公司的名称合成路径
	if(al_IsFileExist(m_szProfile) == FALSE)
	{
		CString str = theApp.m_szMobileName_ori;
		int nth = str.Find(' ');
		CString strCompanyName = str.Left(nth);
		wsprintf(m_szProfile, _T("%s%sAddDlg_%s.ini"), theApp.m_szSkin, _T("Calendar\\"),strCompanyName);
		//cm31使用的ini文件 s60.ini
		if(strCompanyName.CompareNoCase(_T("Nokia")) == 0 && _tcsncicmp(theApp.m_szConnectMode,_T("SYNCML"),6) == 0)
		{
			wsprintf(m_szProfile, _T("%s%sAddDlg_%sS60.ini"), theApp.m_szSkin, _T("Calendar\\"),strCompanyName);
		}
		//默认的路径
		if(al_IsFileExist(m_szProfile) == FALSE)
			wsprintf(m_szProfile, _T("%s%s"), theApp.m_szSkin, _T("Calendar\\AddDlg.ini"));
	}
	//如果指定dlg的ini文件 继续合成，但是本工程没有使用
	TCHAR szReference[MAX_PATH];
	if(al_GetSettingString(_T("Dlg"),_T("ReferenceTO"),m_szProfile,szReference))
	{
		wsprintf(m_szProfile,_T("%sCalendar\\%s"), theApp.m_szSkin,szReference);
	}

	//Resize Window by rect setting
	GetWindowRect(rect);
	GetClientRect(rect2);
	al_GetSettingRect( _T("panel"), _T("rect"), m_szProfile, rect3 );
	rect2.right = rect.Width() + rect3.Width() - rect2.right;
	rect2.bottom = rect.Height() + rect3.Height() - rect2.bottom;
	MoveWindow( &rect2 );	
	CenterWindow();

	//load panel color
	if( !m_hBrush && al_GetSettingColor( _T("panel"), _T("brush"), m_szProfile, color ) )
		m_hBrush = CreateSolidBrush(color);	

	//m_stLtType
	//文字 类型 的格式与大小
	m_stLtType.SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
	m_stLtType.SetBrush(m_hBrush);
	al_GetSettingRect( _T("st_TYPE"), _T("txtLtType_rect"), m_szProfile, rect );
	m_stLtType.MoveWindow(&rect);
	if(al_GetSettingInt(_T("st_TYPE"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_stLtType.ShowWindow(SW_SHOW);
		else
			m_stLtType.ShowWindow(SW_HIDE);

	}

	//下面实现几个static的位置设置  ini中没有要显示的文字，文字信息是在dlg的init中实现的
	//m_stLtTitle	主题		
	m_stLtTitle.SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
	m_stLtTitle.SetBrush(m_hBrush);
	al_GetSettingRect( _T("st_Title"), _T("txtLtTitle_rect"), m_szProfile, rect );
	m_stLtTitle.MoveWindow(&rect);
	if(al_GetSettingInt(_T("st_Title"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_stLtTitle.ShowWindow(SW_SHOW);
		else
			m_stLtTitle.ShowWindow(SW_HIDE);

	}
	//m_stAlarmPhoneNo	地址
	m_stAlarmPhoneNo.SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
	m_stAlarmPhoneNo.SetBrush(m_hBrush);
	al_GetSettingRect( _T("st_Location"), _T("txtAlarmPhoneNo_rect"), m_szProfile, rect );
	m_stAlarmPhoneNo.MoveWindow(&rect);
	if(al_GetSettingInt(_T("st_Location"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_stAlarmPhoneNo.ShowWindow(SW_SHOW);
		else
			m_stAlarmPhoneNo.ShowWindow(SW_HIDE);

	}
	//m_stStartTime  开始时间
	m_stStartTime.SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
	m_stStartTime.SetBrush(m_hBrush);
	al_GetSettingRect( _T("st_StartTime"), _T("txtStartTime_rect"), m_szProfile, rect );
	m_stStartTime.MoveWindow(&rect);
	if(al_GetSettingInt(_T("st_StartTime"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_stStartTime.ShowWindow(SW_SHOW);
		else
			m_stStartTime.ShowWindow(SW_HIDE);

	}
	//m_stStopTime   结束时间
	m_stStopTime.SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
	m_stStopTime.SetBrush(m_hBrush);
	al_GetSettingRect( _T("st_StopTime"), _T("txtStopTime_rect"), m_szProfile, rect );
	m_stStopTime.MoveWindow(&rect);
	if(al_GetSettingInt(_T("st_StopTime"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_stStopTime.ShowWindow(SW_SHOW);
		else
			m_stStopTime.ShowWindow(SW_HIDE);

	}
 //080612libaoliu
	//优先级文字
	m_stPriority.SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
	m_stPriority.SetBrush(m_hBrush);
	al_GetSettingRect( _T("st_Priority"), _T("txtPriority_rect"), m_szProfile, rect );
	m_stPriority.MoveWindow(&rect);
	if(al_GetSettingInt(_T("st_Priority"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_stPriority.ShowWindow(SW_SHOW);
		else
			m_stPriority.ShowWindow(SW_HIDE);	
	}
	// 优先级选择框
	CWnd* pWnd = GetDlgItem(IDC_CMB_CAL_PRIORITY);
	al_GetSettingRect( _T("cmb_Priority"), _T("rect"), m_szProfile, rect );
	pWnd->MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmb_Priority"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			pWnd->ShowWindow(SW_SHOW);
		else
			pWnd->ShowWindow(SW_HIDE);
	}
	else pWnd->ShowWindow(SW_HIDE);


	//load m_chkAlarm   闹铃选择框
	m_chkAlarm.ModifyStyle(0,BS_OWNERDRAW);
	GetCheckFromSetting(&m_chkAlarm, _T("CHK_Alarm"), NULL, NULL, m_szProfile);
	if(al_GetSettingInt(_T("CHK_Alarm"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_chkAlarm.ShowWindow(SW_SHOW);
		else
			m_chkAlarm.ShowWindow(SW_HIDE);
	}
	//load m_chkRepeat  重复选择框
	m_chkRepeat.ModifyStyle(0,BS_OWNERDRAW);
	m_chkRepeat.SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
	GetCheckFromSetting(&m_chkRepeat, _T("CHK_Repeat"), NULL, NULL, m_szProfile);
	if(al_GetSettingInt(_T("CHK_Repeat"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_chkRepeat.ShowWindow(SW_SHOW);
		else
			m_chkRepeat.ShowWindow(SW_HIDE);
	}


	//load Repeat end data  结束时间文字
	m_stRepeatEndData.SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
	m_stRepeatEndData.SetBrush(m_hBrush);
	al_GetSettingRect( _T("st_RepeatEndDate"), _T("rect"), m_szProfile, rect );
	m_stRepeatEndData.MoveWindow(&rect);
	if(al_GetSettingInt(_T("st_RepeatEndDate"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_stRepeatEndData.ShowWindow(SW_SHOW);
		else
			m_stRepeatEndData.ShowWindow(SW_HIDE);

	}
	else m_stRepeatEndData.ShowWindow(SW_HIDE);
	//check   没有显示
	m_chkRepeatEndDate.ModifyStyle(0,BS_OWNERDRAW);
	m_chkRepeatEndDate.SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
	GetCheckFromSetting(&m_chkRepeatEndDate, _T("CHK_RepeatEndDate"), NULL, NULL, m_szProfile);
	if(al_GetSettingInt(_T("CHK_RepeatEndDate"),_T("bVisible"),m_szProfile,bVisible))
	{
		al_GetSettingString(_T("public"),_T("DLG_STATIC_REPEATENDDATA"),theApp.m_szRes,szText);
		m_chkRepeatEndDate.SetWindowText(szText);
		if(bVisible)
			m_chkRepeatEndDate.ShowWindow(SW_SHOW);
		else
			m_chkRepeatEndDate.ShowWindow(SW_HIDE);
	}
	else m_chkRepeatEndDate.ShowWindow(SW_HIDE);
	//结束重复组合框
	CWnd* pWnd2 = GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE);
	al_GetSettingRect( _T("cmb_RepeatEndDate"), _T("rect"), m_szProfile, rect );
	pWnd2->MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmb_RepeatEndDate"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			pWnd2->ShowWindow(SW_SHOW);
		else
			pWnd2->ShowWindow(SW_HIDE);
	}
	else pWnd2->ShowWindow(SW_HIDE);
	//load m_ctrlOk	 保存按钮
	m_ctrlOk.ModifyStyle(0,BS_OWNERDRAW);
	GetButtonFromSetting1(&m_ctrlOk, _T("BTN_SAVE"), NULL, NULL, m_szProfile);

	//load m_ctrlCancel   取消按钮
	m_ctrlCancel.ModifyStyle(0,BS_OWNERDRAW);
	GetButtonFromSetting1(&m_ctrlCancel, _T("BTN_CANCEL"), NULL, NULL, m_szProfile);

	//load m_ckTone  没有显示
	m_ckTone.ModifyStyle(0,BS_OWNERDRAW);
	GetCheckFromSetting(&m_ckTone, _T("CHK_Tone"), NULL, NULL, m_szProfile);
	if(al_GetSettingInt(_T("CHK_Tone"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_ckTone.ShowWindow(SW_SHOW);
		else
			m_ckTone.ShowWindow(SW_HIDE);
	}

	//load m_cbNoteType   类型组合框
	al_GetSettingRect( _T("cmb_NoteType"), _T("cmbNoteType_rect"), m_szProfile, rect );
	m_cbNoteType.MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmb_NoteType"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_cbNoteType.ShowWindow(SW_SHOW);
		else
			m_cbNoteType.ShowWindow(SW_HIDE);

	}


	//load IDC_SCH_DATA_TITLE  主题编辑框
	al_GetSettingRect( _T("edt_Title"), _T("edtDataTitle_rect"), m_szProfile, rect );
	GetDlgItem(IDC_SCH_DATA_TITLE)->MoveWindow(&rect);
	if(al_GetSettingInt(_T("edt_Title"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			GetDlgItem(IDC_SCH_DATA_TITLE)->ShowWindow(SW_SHOW);
		else
			GetDlgItem(IDC_SCH_DATA_TITLE)->ShowWindow(SW_HIDE);

	}

	//load m_etLocation   地址编辑框
	 //080612libaoliu
	al_GetSettingRect( _T("edt_Location"), _T("edtLocation_rect"), m_szProfile, rect );
	m_etLocation.MoveWindow(&rect);
	GetDlgItem(IDC_SCH_DATA_ALARM_PHONENO)->MoveWindow(&rect);
	if(al_GetSettingInt(_T("edt_Location"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
		{
			m_etLocation.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_SCH_DATA_ALARM_PHONENO)->ShowWindow(SW_SHOW);
		}
		else
		{
			
			m_etLocation.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SCH_DATA_ALARM_PHONENO)->ShowWindow(SW_HIDE);
		}

	}
	

	//load m_cbStartDate   起始时间组合框1
	al_GetSettingRect( _T("cmb_startDate"), _T("cbStartDate_rect"), m_szProfile, rect );
	m_cbStartDate.MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmb_startDate"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_cbStartDate.ShowWindow(SW_SHOW);
		else
			m_cbStartDate.ShowWindow(SW_HIDE);

	}

	//load m_CbStartTime   起始时间组合框2
	al_GetSettingRect( _T("cmb_Startime"), _T("cbStartTime_rect"), m_szProfile, rect );
	m_CbStartTime.MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmb_Startime"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
		{
			m_CbStartTime.ShowWindow(SW_SHOW);
		}
		else
			m_CbStartTime.ShowWindow(SW_HIDE);

	}

	//load m_cbStopDate  结束时间1
	al_GetSettingRect( _T("cmb_StopDate"), _T("cbStopDate_rect"), m_szProfile, rect );
	m_cbStopDate.MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmb_StopDate"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_cbStopDate.ShowWindow(SW_SHOW);
		else
			m_cbStopDate.ShowWindow(SW_HIDE);

	}

	//load m_CbStopTime  结束时间2
	al_GetSettingRect( _T("cmb_StopTime"), _T("cbStopTime_rect"), m_szProfile, rect );
	m_CbStopTime.MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmb_StopTime"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_CbStopTime.ShowWindow(SW_SHOW);
		else
			m_CbStopTime.ShowWindow(SW_HIDE);
	}
	//没用到
	al_GetSettingRect( _T("edt_StopTime"), _T("rect"), m_szProfile, rect );
	m_edtStopTime.MoveWindow(&rect);
	if(al_GetSettingInt(_T("edt_StopTime"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_edtStopTime.ShowWindow(SW_SHOW);
		else
			m_edtStopTime.ShowWindow(SW_HIDE);
	}
	//没用到
	al_GetSettingRect( _T("cmd_StopTimeType"), _T("rect"), m_szProfile, rect );
	m_cbStopTimeType.MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmd_StopTimeType"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_cbStopTimeType.ShowWindow(SW_SHOW);
		else
			m_cbStopTimeType.ShowWindow(SW_HIDE);
	}

	//修改为可用，替换原来闹铃得两个组合框
	//load m_cbAlarmLeadTime  
	al_GetSettingRect( _T("cmb_AlarmLeadTime"), _T("cbAlarmLeadTime_rect"), m_szProfile, rect );
	m_cbAlarmLeadTime.MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmb_AlarmLeadTime"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_cbAlarmLeadTime.ShowWindow(SW_SHOW);
		else
			m_cbAlarmLeadTime.ShowWindow(SW_HIDE);
	}
	//不使用下面两个组合框 在INI中修改
	//load m_cbAlarmday   闹铃组合框1
	al_GetSettingRect( _T("cmb_Alarmday"), _T("cbAlarmday_rect"), m_szProfile, rect );
	m_cbAlarmday.MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmb_Alarmday"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_cbAlarmday.ShowWindow(SW_SHOW);
		else
			m_cbAlarmday.ShowWindow(SW_HIDE);
	}

	//load m_cbAlarmTime  闹铃组合框2
	al_GetSettingRect( _T("cmb_AlarmTime"), _T("cbAlarmTime_rect"), m_szProfile, rect );
	m_cbAlarmTime.MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmb_AlarmTime"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_cbAlarmTime.ShowWindow(SW_SHOW);
		else
			m_cbAlarmTime.ShowWindow(SW_HIDE);
	}

	al_GetSettingRect( _T("edt_AlarmTime"), _T("rect"), m_szProfile, rect );
	m_edtAlarmTime.MoveWindow(&rect);
	if(al_GetSettingInt(_T("edt_AlarmTime"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_edtAlarmTime.ShowWindow(SW_SHOW);
		else
			m_edtAlarmTime.ShowWindow(SW_HIDE);
	}

	al_GetSettingRect( _T("cmb_AlarmTimeType"), _T("rect"), m_szProfile, rect );
	m_cbAlarmTimeType.MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmb_AlarmTimeType"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_cbAlarmTimeType.ShowWindow(SW_SHOW);
		else
			m_cbAlarmTimeType.ShowWindow(SW_HIDE);
	}

	//load m_cbRepeatType  重复类型组合框
	al_GetSettingRect( _T("cmb_RepeatType"), _T("cbRepeatType_rect"), m_szProfile, rect );
	m_cbRepeatType.MoveWindow(&rect);
	if(al_GetSettingInt(_T("cmb_RepeatType"),_T("bVisible"),m_szProfile,bVisible))
	{
		if(bVisible)
			m_cbRepeatType.ShowWindow(SW_SHOW);
		else
			m_cbRepeatType.ShowWindow(SW_HIDE);
	}

	//m_stDescription   没用到
	m_stDescription.SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
	m_stDescription.SetBrush(m_hBrush);
	if(al_GetSettingRect( _T("st_description"), _T("rect"), m_szProfile, rect ))
	{
		m_stDescription.MoveWindow(&rect);
		if(al_GetSettingInt(_T("st_description"),_T("bVisible"),m_szProfile,bVisible))
		{
			if(bVisible)
				m_stDescription.ShowWindow(SW_SHOW);
			else
				m_stDescription.ShowWindow(SW_HIDE);

		}
	}
	else m_stDescription.ShowWindow(SW_HIDE);

	//load m_edDescription
	if(al_GetSettingRect( _T("edt_Description"), _T("rect"), m_szProfile, rect ))
	{
		m_edDescription.MoveWindow(&rect);
		if(al_GetSettingInt(_T("edt_Description"),_T("bVisible"),m_szProfile,bVisible))
		{
			if(bVisible)
				m_edDescription.ShowWindow(SW_SHOW);
			else
				m_edDescription.ShowWindow(SW_HIDE);

		}
	}
	else m_edDescription.ShowWindow(SW_HIDE);

	//load DayofWeek   使用之， 初始化7个星期选择复选按钮
	 //080612libaoliu
	CCheckEx2	*pCheck[7] = {&m_chWeek1,&m_chWeek2,&m_chWeek3,&m_chWeek4,&m_chWeek5,&m_chWeek6,&m_chWeek7};
	TCHAR szChSec[7][32] = {_T("ch_Week1"),_T("ch_Week2"),_T("ch_Week3"),_T("ch_Week4"),_T("ch_Week5"),_T("ch_Week6"),_T("ch_Week7")};
	TCHAR szChKEY[7][32] = {_T("IDS_SUN"),_T("IDS_MON"),_T("IDS_TUE"),_T("IDS_WED"),_T("IDS_THU"),_T("IDS_FRI"),_T("IDS_SAT")};
	for(int i = 0 ; i < 7 ; i ++)
	{
		pCheck[i]->ModifyStyle(0,BS_OWNERDRAW);
		pCheck[i]->SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
		GetCheckFromSetting(pCheck[i], szChSec[i], szChKEY[i], NULL, m_szProfile);
		al_GetSettingString(_T("public"),szChKEY[i],theApp.m_szRes,szText);
		pCheck[i]->ShowText(TRUE);
		pCheck[i]->SetWindowText(szText);

		if(al_GetSettingInt(szChSec[i],_T("bVisible"),m_szProfile,bVisible))
		{
			if(bVisible)
				pCheck[i]->ShowWindow(SW_SHOW);
			else
				pCheck[i]->ShowWindow(SW_HIDE);

		}
		else pCheck[i]->ShowWindow(SW_HIDE);
	}


	//m_scRepeatON   没用到
	m_scRepeatON.SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
	m_scRepeatON.SetBrush(m_hBrush);
	if(al_GetSettingRect( _T("st_RepeatOn"), _T("rect"), m_szProfile, rect ))
	{
		m_scRepeatON.MoveWindow(&rect);
		al_GetSettingString(_T("public"), _T("IDS_REPEATON"),theApp.m_szRes,szText);
		m_scRepeatON.SetWindowText(szText);

		if(al_GetSettingInt( _T("st_RepeatOn"),_T("bVisible"),m_szProfile,bVisible))
		{
			if(bVisible)
				m_scRepeatON.ShowWindow(SW_SHOW);
			else
				m_scRepeatON.ShowWindow(SW_HIDE);

		}
	}
	else m_scRepeatON.ShowWindow(SW_HIDE);
	//m_scRepeatinterval
	m_scRepeatinterval.SetTextFont(szFontName, nFontSize, FALSE, ANTIALIASED_QUALITY);
	m_scRepeatinterval.SetBrush(m_hBrush);
	if(al_GetSettingRect( _T("st_Repeatinterval"), _T("rect"), m_szProfile, rect ))
	{
		m_scRepeatinterval.MoveWindow(&rect);
		al_GetSettingString(_T("public"),_T("IDS_REPEATINTERVAL"),theApp.m_szRes,szText);
		m_scRepeatinterval.SetWindowText(szText);
		if(al_GetSettingInt(_T("st_Repeatinterval"),_T("bVisible"),m_szProfile,bVisible))
		{
			if(bVisible)
				m_scRepeatinterval.ShowWindow(SW_SHOW);
			else
				m_scRepeatinterval.ShowWindow(SW_HIDE);

		}
	}
	else m_scRepeatinterval.ShowWindow(SW_HIDE);

	//load m_edRepeatInterval  没用到
	if(al_GetSettingRect( _T("ed_RepeatInterval"), _T("rect"), m_szProfile, rect ))
	{
		rect.right -=11;
		m_edRepeatInterval.MoveWindow(&rect);
		rect.left = rect.right+1;
		rect.right = rect.left +10;

		m_spRepeatinterval.MoveWindow(&rect);
		if(al_GetSettingInt(_T("ed_RepeatInterval"),_T("bVisible"),m_szProfile,bVisible))
		{
			m_spRepeatinterval.SetBuddy(&m_edRepeatInterval);
			if(bVisible)
			{
				m_edRepeatInterval.ShowWindow(SW_SHOW);
				m_spRepeatinterval.ShowWindow(SW_SHOW);
			}
			else
			{
				m_edRepeatInterval.ShowWindow(SW_HIDE);
				m_spRepeatinterval.ShowWindow(SW_HIDE);
			}
			int nMax = 1;
			al_GetSettingInt(_T("ed_RepeatInterval"),_T("Max"),m_szProfile,nMax);
			m_spRepeatinterval.SetRange(1,nMax);
			m_edRepeatInterval.SetWindowText(_T("1"));
		}
		else
		{
			m_edRepeatInterval.ShowWindow(SW_HIDE);
			m_spRepeatinterval.ShowWindow(SW_HIDE);
		}
	}
	else
	{
		m_edRepeatInterval.ShowWindow(SW_HIDE);
		m_spRepeatinterval.ShowWindow(SW_HIDE);
	}
}

void CCAMMScheduleEditorDlg::OnDatetimechangeSchDataStartdate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	COleDateTime olestartDate ,oleAlarmDate,oleTemp;
	olestartDate.SetDateTime(m_StartDate.GetYear(),m_StartDate.GetMonth(),m_StartDate.GetDay(),0,0,0);
	oleAlarmDate.SetDateTime(m_Alarmday.GetYear(),m_Alarmday.GetMonth(),m_Alarmday.GetDay(),0,0,0);
	oleTemp = olestartDate-oleAlarmDate;

	xUpdateData(true);
	if(!::IsWindowEnabled(m_cbStopDate.m_hWnd) || (m_cbStopDate.GetStyle() & WS_VISIBLE) == FALSE)
		m_StopDate = m_StartDate;
	else
	{
		if(m_StopDate<m_StartDate)
			m_StopDate = m_StartDate;

	}

	//如果开始结束日期不相同，禁止重复  kerm add for 9a9u 1.16
	if(m_StopDate != m_StartDate)
	{
		m_chkRepeat.EnableWindow(false);
		m_chkRepeat.SetCheck(false);
		m_cbRepeatType.SetCurSel(0);
		m_cbRepeatType.EnableWindow(false);
		m_chkAlarm.SetCheck(false);
		m_chkAlarm.EnableWindow(false);
		m_cbAlarmLeadTime.SetCurSel(0);	
		m_cbAlarmLeadTime.EnableWindow(false);
		
		m_bAlarm = false;
		m_bRepeat = false;
	}
	else
	{
		if (m_cbNoteType.GetCurSel()==3)//080526libaoliu
		{
		m_chkRepeat.EnableWindow(false);
		}
		else
		m_chkRepeat.EnableWindow(true);
		m_chkAlarm.EnableWindow(true);
	}

//	if(!::IsWindowEnabled(m_cbAlarmday.m_hWnd))
//		m_Alarmday = m_StartDate;
	if(oleTemp.m_dt>0)
		m_Alarmday = m_StartDate - oleTemp;
	else
		m_Alarmday = m_StartDate;

	xUpdateData(false);
	UpdateDateofWeekCtlStates();

	*pResult = 0;
}
//kerm add for 9a9u 1.16
void CCAMMScheduleEditorDlg::OnDatetimechangeSchDataEnddate(NMHDR* pNMHDR, LRESULT* pResult) 
{	
	COleDateTime olestartDate ,oleAlarmDate,oleTemp;
	olestartDate.SetDateTime(m_StartDate.GetYear(),m_StartDate.GetMonth(),m_StartDate.GetDay(),0,0,0);
	oleAlarmDate.SetDateTime(m_Alarmday.GetYear(),m_Alarmday.GetMonth(),m_Alarmday.GetDay(),0,0,0);
	oleTemp = olestartDate-oleAlarmDate;
	
	xUpdateData(true);
	if(m_StopDate<m_StartDate)
			m_StopDate = m_StartDate;
	//如果开始结束日期不相同，禁止重复  kerm add for 9a9u 1.16
	if(m_StopDate != m_StartDate)
	{
		m_chkRepeat.EnableWindow(false);
		m_chkRepeat.SetCheck(false);
		m_cbRepeatType.SetCurSel(0);
		m_cbRepeatType.EnableWindow(false);
		m_chkAlarm.SetCheck(false);
		m_chkAlarm.EnableWindow(false);
		m_cbAlarmLeadTime.SetCurSel(0);	
		m_cbAlarmLeadTime.EnableWindow(false);
		m_bAlarm = false;
		m_bRepeat = false;
	}
	else
	{
		m_chkRepeat.EnableWindow(true);
		m_chkAlarm.EnableWindow(true);
	}
	
	//	if(!::IsWindowEnabled(m_cbAlarmday.m_hWnd))
	//		m_Alarmday = m_StartDate;
	
	xUpdateData(false);
	UpdateDateofWeekCtlStates();
	
	*pResult = 0;
}


DWORD CCAMMScheduleEditorDlg::GetTimeInterval(CString strStartStr,CString strEndStr )
{
	CScheduleTimeInfo StartTimeInfo ,StopTimeInfo ;
	int hour,min;
	StartTimeInfo.ParseAMPMTimeString(strStartStr);
	StartTimeInfo.Get24HourTime(hour,min);
	COleDateTimeSpan dtStartTimeSpan(0,hour,min,0);

	StopTimeInfo.ParseAMPMTimeString(strEndStr);
	StopTimeInfo.Get24HourTime(hour,min);
	COleDateTimeSpan dtStopTimeSpan(0,hour,min,0);

	COleDateTimeSpan dtTimeSpan = dtStopTimeSpan - dtStartTimeSpan  ;
	return (DWORD)(dtTimeSpan.GetTotalMinutes());
}
DWORD CCAMMScheduleEditorDlg::GetStartEndTimeInterval()
{
	TCHAR hourStartStr[50];
	TCHAR hourEndStr[50];
	m_CbStartTime.GetWindowText(hourStartStr,50);
	m_CbStopTime.GetWindowText(hourEndStr,50);

	CScheduleTimeInfo StartTimeInfo ,StopTimeInfo ;
	int hour,min;
	StartTimeInfo.ParseAMPMTimeString(hourStartStr);
	StartTimeInfo.Get24HourTime(hour,min);
	COleDateTimeSpan dtStartTimeSpan(0,hour,min,0);

	StopTimeInfo.ParseAMPMTimeString(hourEndStr);
	StopTimeInfo.Get24HourTime(hour,min);
	COleDateTimeSpan dtStopTimeSpan(0,hour,min,0);
	COleDateTimeSpan dt30min(0,0,30,0);

	while(dtStopTimeSpan < dtStartTimeSpan)
	{
		dtStopTimeSpan += dt30min;
	}
	COleDateTimeSpan dtTimeSpan = dtStopTimeSpan - dtStartTimeSpan  ;
	return (DWORD)(dtTimeSpan.GetTotalMinutes());
}


void CCAMMScheduleEditorDlg::OnKillfocusSchDataStarttime() 
{
	// TODO: Add your control notification handler code here
	CScheduleTimeInfo stiStartTime;
	TCHAR txt[MAX_LENGTH];
	memset(txt,0x00,MAX_LENGTH);
	m_CbStartTime.GetWindowText(txt,MAX_LENGTH);

	CString txtStr=txt;
	if (!stiStartTime.ParseAMPMTimeString(txtStr))
	{
		CalendarMessageBox(m_hWnd,_T("MSG_ERROR_TIMEFORMAT"));
		m_CbStartTime.SetFocus();
		return;
	}

	CString strTime = txtStr.Left(txtStr.GetLength() -2);
	if(stiStartTime.min != 0 && stiStartTime.min !=30)
	{
		if(stiStartTime.min > 30)
			strTime += "30";
		else
			strTime += "00";
	}
	else
		strTime = txtStr;


	TCHAR hourStr[50];
	for (int i=0  ;i<m_CbStartTime.GetCount();i++) {
		m_CbStartTime.GetLBText(i,hourStr);
		if(strTime.CompareNoCase(hourStr) == 0)
			break;
	}
	m_CbStartTime.SetCurSel(i);
	m_CbStartTime.SetWindowText(txtStr);

	DWORD dwTimeInter = GetStartEndTimeInterval();
	int nIndex = dwTimeInter/30;

	xUpdateData(TRUE);
	
//	UpdateStopTime(dwTimeInter,m_CbStartTime.GetCurSel());

	xUpdateStopTimeCb(m_CbStartTime.GetCurSel());


	int nStopIndex = 0;
	if(m_CbStopTime.IsWindowEnabled())
	{
		nStopIndex = max(0,nIndex);
	}
	else
		nStopIndex = 0;

	TCHAR hourStopStr[50];
	m_CbStopTime.GetLBText(nStopIndex,hourStopStr);
	m_CbStopTime.GetLBText(nStopIndex,hourStopStr);
	CScheduleTimeInfo StartTimeInfo ,StopTimeInfo ;
	StartTimeInfo.ParseAMPMTimeString(txtStr);
	StopTimeInfo.ParseAMPMTimeString(hourStopStr);
	if(StopTimeInfo < StartTimeInfo)
		nStopIndex++;


	m_CbStopTime.SetCurSel(nStopIndex);

//	OnSelchangeAlarmInterval();	
	PostMessage(WM_COMMAND ,MAKEWPARAM(IDC_SCH_DATA_ALARM_INTERVAL,CBN_SELCHANGE),(LPARAM)m_cbAlarmLeadTime.m_hWnd);
	xUpdateData(FALSE);
	
}

void CCAMMScheduleEditorDlg::OnKillfocusSchDataStoptime() 
{
	// TODO: Add your control notification handler code here
	CScheduleTimeInfo stiStopTime;
	TCHAR txt[MAX_LENGTH];
	memset(txt,0x00,MAX_LENGTH);
	m_CbStopTime.GetWindowText(txt,MAX_LENGTH);

	CString txtStr=txt;
	if (!stiStopTime.ParseAMPMTimeString(txtStr))
	{
		CalendarMessageBox(m_hWnd,_T("MSG_ERROR_TIMEFORMAT"));
		m_CbStopTime.SetFocus();
		return;
	}
	
}
// 1:daily 2:Weekly 3: two weekly 4: monthly 5 : Monthly by week 6:yearly
void CCAMMScheduleEditorDlg::InsertRepeatTypetoComboCtrl(DWORD dwFalg)
{//AfxMessageBox(L"准备插入重复类型");
	if(m_dwRepeatTypeFlag== dwFalg)
		return;
 	m_cbRepeatType.ResetContent();
    TCHAR  szRepeatType[MAX_PATH];
	int nIndex;
	if((dwFalg>>1) &1)//daily
	{
    	al_GetSettingString(_T("repeattype"), _T("0"), theApp.m_szRes,szRepeatType);
    	nIndex = m_cbRepeatType.AddString(szRepeatType);
		m_cbRepeatType.SetItemData(nIndex,DAILY);
		//test
// 		CString st;
// 		st.Format(L"%d",m_cbRepeatType.GetItemData(nIndex) );
// 		AfxMessageBox(st);
	}

	if((dwFalg>>2) &1) //Weekly
	{
    	al_GetSettingString(_T("repeattype"), _T("1"), theApp.m_szRes,szRepeatType);//Weekly
    	nIndex = m_cbRepeatType.AddString(szRepeatType);
		m_cbRepeatType.SetItemData(nIndex,WEEKLY);
		//test
// 		CString st;
// 		st.Format(L"%d",m_cbRepeatType.GetItemData(nIndex) );
// 		AfxMessageBox(st);
	}

	if((dwFalg>>3) &1) //two Weekly
	{
    	al_GetSettingString(_T("repeattype"), _T("2"), theApp.m_szRes,szRepeatType);//two Weekly
    	nIndex = m_cbRepeatType.AddString(szRepeatType);
		m_cbRepeatType.SetItemData(nIndex,TWOWEEKLY);
		/*AfxMessageBox(L"2");*/
	}
	if((dwFalg>>4) &1) //monthly
	{
    	al_GetSettingString(_T("repeattype"), _T("3"), theApp.m_szRes,szRepeatType);//monthly
    	nIndex = m_cbRepeatType.AddString(szRepeatType);
		m_cbRepeatType.SetItemData(nIndex,MONTHLY);
		//test
// 		CString st;
// 		st.Format(L"%d",m_cbRepeatType.GetItemData(nIndex) );
// 		AfxMessageBox(st);
	}
	if((dwFalg>>5) &1) //Monthly by week
	{
    	al_GetSettingString(_T("repeattype"), _T("5"), theApp.m_szRes,szRepeatType);//Monthly by week
    	nIndex = m_cbRepeatType.AddString(szRepeatType);
		m_cbRepeatType.SetItemData(nIndex,WEEKLYMONTHLY);
	/*	AfxMessageBox(L"5");*/
	}
	if((dwFalg>>6) &1) //yearly
	{
    	al_GetSettingString(_T("repeattype"), _T("4"), theApp.m_szRes,szRepeatType);//yearly
    	nIndex = m_cbRepeatType.AddString(szRepeatType);
		m_cbRepeatType.SetItemData(nIndex,YEARLY);
		/*AfxMessageBox(L"4");*/
	}
	if((dwFalg>>7) &1) //weekend
	{
    	al_GetSettingString(_T("repeattype"), _T("7"), theApp.m_szRes,szRepeatType);//weekend
    	nIndex = m_cbRepeatType.AddString(szRepeatType);
		m_cbRepeatType.SetItemData(nIndex,WEEKEND);
	/*	AfxMessageBox(L"7");*/
	}
	if((dwFalg>>8) &1) //weekdays
	{
    	al_GetSettingString(_T("repeattype"), _T("8"), theApp.m_szRes,szRepeatType);//weekdays
    	nIndex = m_cbRepeatType.AddString(szRepeatType);
		m_cbRepeatType.SetItemData(nIndex,WEEKDAYS);
		//test
// 		CString st;
// 		st.Format(L"%d",m_cbRepeatType.GetItemData(nIndex) );
// 		AfxMessageBox(st);
	}
	m_dwRepeatTypeFlag= dwFalg;
	m_cbRepeatType.SetCurSel(0);
	SetRepeatType(m_ScheduleInfo.repeatType , m_ScheduleInfo.nDayofWeekMask);

}
//0: anther time ,1 : 0 min ,2 : 5 min ,3 : 10 min,4 : 15 min,5 : 20 min,6 : 25 min ,...,12: 55Min
//13: 1h ,14 :2h ,15:3h,16:4h,17:5h ,18:6h,19 : 7h ,20:8h ,21:9h , 22:10h, 23:11h,24:12h
//25: 1D, 26:2D,27 :3d,28:4d,29:5d,30:6d,31:7d

void CCAMMScheduleEditorDlg::InsertAlarmLeadTypetoComboCtrl(DWORD dwFalg)
{
	if(	m_dwAlarmLeadTypeFlag == dwFalg)
		return;

	m_cbAlarmLeadTime.ResetContent();
	CString strmin = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_MIN"));
	CString strHour = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_HOUR"));
	CString strDay = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_DAY"));

	int nIndex;
	int i;
	CString StringLoaded;
	//下面代码向闹铃组合框中插入内容，for 9@9u 只需要提前0 5 15 30分钟提醒 直接插入
	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_OPEN"));
	nIndex = m_cbAlarmLeadTime.AddString(StringLoaded);
	m_cbAlarmLeadTime.SetItemData(nIndex,0*5);	//提前0分钟，表示为开
	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_BEFORE5"));
	nIndex = m_cbAlarmLeadTime.AddString(StringLoaded);
	m_cbAlarmLeadTime.SetItemData(nIndex,1*5);
	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_BEFORE10"));
	nIndex = m_cbAlarmLeadTime.AddString(StringLoaded);
	m_cbAlarmLeadTime.SetItemData(nIndex,2*5);
	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_BEFOREHOUR"));
	nIndex = m_cbAlarmLeadTime.AddString(StringLoaded);
	m_cbAlarmLeadTime.SetItemData(nIndex,12*5);
	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_BEFOREDAY"));
	nIndex = m_cbAlarmLeadTime.AddString(StringLoaded);
	m_cbAlarmLeadTime.SetItemData(nIndex,1440);
	StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_BEFOREWEEK"));
	nIndex = m_cbAlarmLeadTime.AddString(StringLoaded);
	m_cbAlarmLeadTime.SetItemData(nIndex,10080);





	
/*
	if(dwFalg &1)
	{
		StringLoaded = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_ANOTHERTIME"));
		nIndex = m_cbAlarmLeadTime.AddString(StringLoaded);
		m_cbAlarmLeadTime.SetItemData(nIndex,65535);
	}
	for(int i = 1 ;i<=12 ; i++)
	{
		if((dwFalg>>i) &1) //(i-1) *5 min
		{
			StringLoaded.Format(_T("%d %s"),(i)*5,strmin);	//从5分钟开始
			nIndex = m_cbAlarmLeadTime.AddString(StringLoaded);
			m_cbAlarmLeadTime.SetItemData(nIndex,(i-1)*5);
		}
	}*/
/*
	for(i = 13 ;i<=24 ; i++)
	{
		if((dwFalg>>i) &1) //(i-12) h
		{
			StringLoaded.Format(_T("%d %s"),(i-12),strHour);
			nIndex = m_cbAlarmLeadTime.AddString(StringLoaded);
			m_cbAlarmLeadTime.SetItemData(nIndex,(i-12) *60);
		}
	}

	for(i = 25 ;i<=31 ; i++)
	{
		if((dwFalg>>i) &1) //(i-24) d
		{
			StringLoaded.Format(_T("%d %s"),(i-24),strDay);
			nIndex = m_cbAlarmLeadTime.AddString(StringLoaded);
			m_cbAlarmLeadTime.SetItemData(nIndex,(i-24)*24*60);
		}
	}
*/
	m_dwAlarmLeadTypeFlag = dwFalg;

	int nHour=0, nMin=0;
	m_ScheduleInfo.hiRealStartTime.Get24HourTime(nHour, nMin);

	COleDateTime dtStart(m_ScheduleInfo.dtStartDate.GetYear(),m_ScheduleInfo.dtStartDate.GetMonth(),m_ScheduleInfo.dtStartDate.GetDay(),
		nHour,nMin,0);
	DWORD dwMin = m_ScheduleInfo.GetAlarmBeforeMin(dtStart,m_ScheduleInfo.dtAlarmDateTime);

	int nCount = m_cbAlarmLeadTime.GetCount();
	for(  i = 0 ; i < nCount ; i++)
	{
		if( m_cbAlarmLeadTime.GetItemData(i) == dwMin)
		{
			m_cbAlarmLeadTime.SetCurSel(i);
			m_nInitAlarmLeadTime = i;
			break;
		}
	}
	if(i>=nCount) 
	{
		m_cbAlarmLeadTime.SetCurSel(0);
		m_nInitAlarmLeadTime = 0;
	}
}

DWORD CCAMMScheduleEditorDlg::GetAlarmLeadTime()
{
	if((m_cbAlarmLeadTime.GetStyle() & WS_VISIBLE))
	{
		int nIndex = m_cbAlarmLeadTime.GetCurSel();
		return m_cbAlarmLeadTime.GetItemData(nIndex);
	}
	return 65535;

}
//0 :sec , 1:min , 2:hour ,3:day ,4:week ,5:month ,6 :Year
void CCAMMScheduleEditorDlg::InsertEndTimeIntervalTypetoComboCtrl(DWORD dwFalg)
{
	if(	m_dwStopTimeIntervalTypeFlag == dwFalg)
		return;

	m_cbStopTimeType.ResetContent();

	CString strSec = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_SEC"));
	CString strmin = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_MIN"));
	CString strHour = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_HOUR"));
	CString strDay = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_DAY"));
	CString strWeek = LoadStringFromFile(_T("public"),_T("IDS_Week"));
	CString strMonth = LoadStringFromFile(_T("public"),_T("IDS_Month"));
	CString strYear = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_YEAR"));
	int nIndex;

	if(dwFalg &1) //sec
	{
		nIndex = m_cbStopTimeType.AddString(strSec);
		m_cbStopTimeType.SetItemData(nIndex,TYPE_SEC);
	}

	if((dwFalg>>1) &1) //min
	{
		nIndex = m_cbStopTimeType.AddString(strmin);
		m_cbStopTimeType.SetItemData(nIndex,TYPE_MIN);
	}
	if((dwFalg>>2) &1) //hour
	{
		nIndex = m_cbStopTimeType.AddString(strHour);
		m_cbStopTimeType.SetItemData(nIndex,TYPE_HOUR);
	}
	if((dwFalg>>3) &1) //day
	{
		nIndex = m_cbStopTimeType.AddString(strDay);
		m_cbStopTimeType.SetItemData(nIndex,TYPE_DAY);
	}
	if((dwFalg>>4) &1) //week
	{
		nIndex = m_cbStopTimeType.AddString(strWeek);
		m_cbStopTimeType.SetItemData(nIndex,TYPE_WEEK);
	}
	if((dwFalg>>5) &1) //month
	{
		nIndex = m_cbStopTimeType.AddString(strMonth);
		m_cbStopTimeType.SetItemData(nIndex,TYPE_MONTH);
	}
	if((dwFalg>>6) &1) //Year
	{
		nIndex = m_cbStopTimeType.AddString(strYear);
		m_cbStopTimeType.SetItemData(nIndex,TYPE_YEAR);
	}

	m_dwStopTimeIntervalTypeFlag = dwFalg;
	m_cbStopTimeType.SetCurSel(0);
}
void CCAMMScheduleEditorDlg::InsertAlarmTimeIntervalTypetoComboCtrl(DWORD dwFalg)
{
	if(	m_dwAlarmTimeIntervalTypeFlag == dwFalg)
		return;
	m_cbAlarmTimeType.ResetContent();
	CString strSec = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_SEC"));
	CString strmin = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_MIN"));
	CString strHour = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_HOUR"));
	CString strDay = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_DAY"));
	CString strWeek = LoadStringFromFile(_T("public"),_T("IDS_Week"));
	CString strMonth = LoadStringFromFile(_T("public"),_T("IDS_Month"));
	CString strYear = LoadStringFromFile(_T("EditDlg"),_T("DLG_STATIC_YEAR"));
	int nIndex;

	if(dwFalg &1) //sec
	{
		nIndex = m_cbAlarmTimeType.AddString(strSec);
		m_cbAlarmTimeType.SetItemData(nIndex,TYPE_SEC);
	}

	if((dwFalg>>1) &1) //min
	{
		nIndex = m_cbAlarmTimeType.AddString(strmin);
		m_cbAlarmTimeType.SetItemData(nIndex,TYPE_MIN);
	}
	if((dwFalg>>2) &1) //hour
	{
		nIndex = m_cbAlarmTimeType.AddString(strHour);
		m_cbAlarmTimeType.SetItemData(nIndex,TYPE_HOUR);
	}
	if((dwFalg>>3) &1) //day
	{
		nIndex = m_cbAlarmTimeType.AddString(strDay);
		m_cbAlarmTimeType.SetItemData(nIndex,TYPE_DAY);
	}
	if((dwFalg>>4) &1) //week
	{
		nIndex = m_cbAlarmTimeType.AddString(strWeek);
		m_cbAlarmTimeType.SetItemData(nIndex,TYPE_WEEK);
	}
	if((dwFalg>>5) &1) //month
	{
		nIndex = m_cbAlarmTimeType.AddString(strMonth);
		m_cbAlarmTimeType.SetItemData(nIndex,TYPE_MONTH);
	}
	if((dwFalg>>6) &1) //Year
	{
		nIndex = m_cbAlarmTimeType.AddString(strYear);
		m_cbAlarmTimeType.SetItemData(nIndex,TYPE_YEAR);
	}

	m_dwAlarmTimeIntervalTypeFlag = dwFalg;
	m_cbAlarmTimeType.SetCurSel(0);
}

BOOL CCAMMScheduleEditorDlg::IsStopDateTimelegal()
{
	BOOL bLegal = FALSE;

	CString txtStr;
	TCHAR txt[MAX_LENGTH];
	CScheduleTimeInfo stiStartTime;
	CScheduleTimeInfo stiStopTime;
	COleDateTime dtStart,dtStop;

	if((m_CbStartTime.GetStyle() & WS_VISIBLE))
	{
		memset(txt,0x00,MAX_LENGTH);
		m_CbStartTime.GetWindowText(txt,MAX_LENGTH);
		CString txtStr=txt;
		if (!stiStartTime.ParseAMPMTimeString(txtStr))
			return bLegal;
	}
	else
		stiStartTime.SetData(9,0,0);

	if((m_cbStartDate.GetStyle() & WS_VISIBLE))
	{
		int nH,nMin;
		stiStartTime.Get24HourTime(nH,nMin);
		dtStart.SetDateTime(m_StartDate.GetYear(),m_StartDate.GetMonth(),m_StartDate.GetDay(),nH,nMin,0);
	}
	else
		return bLegal;



	if((m_CbStopTime.GetStyle() & WS_VISIBLE) ||(m_cbStopDate.GetStyle() & WS_VISIBLE) )
	{
		if((m_CbStopTime.GetStyle() & WS_VISIBLE))
		{
			memset(txt,0x00,MAX_LENGTH);
			m_CbStopTime.GetWindowText(txt,MAX_LENGTH);
			txtStr=txt;
			if (!stiStopTime.ParseAMPMTimeString(txtStr)) 
				return bLegal;
		}
		else
			stiStopTime = stiStartTime;

		if((m_cbStopDate.GetStyle() & WS_VISIBLE))
		{
			int nH,nMin;
			stiStopTime.Get24HourTime(nH,nMin);
			dtStop.SetDateTime(m_StopDate.GetYear(),m_StopDate.GetMonth(),m_StopDate.GetDay(),nH,nMin,0);
		}
		else
		{
			int nH,nMin;
			stiStopTime.Get24HourTime(nH,nMin);
			dtStop.SetDateTime(m_StartDate.GetYear(),m_StartDate.GetMonth(),m_StartDate.GetDay(),nH,nMin,0);
		}


	}
	else if((m_cbStopTimeType.GetStyle() & WS_VISIBLE)  && (m_edtStopTime.GetStyle() & WS_VISIBLE))
	{
		COleDateTime Interval;	
		TimeLengthType type =(TimeLengthType)(m_cbStopTimeType.GetItemData( m_cbStopTimeType.GetCurSel()));
		switch(type)
		{
		case TYPE_SEC:
			{
				COleDateTimeSpan timeSpan(0,0,0,m_nStopLength);
				dtStop = dtStart + timeSpan;
			}
			break;
		case TYPE_MIN:
			{
				COleDateTimeSpan timeSpan(0,0,m_nStopLength,0);
				dtStop = dtStart + timeSpan;
			}
			break;
		case TYPE_HOUR:
			{
				COleDateTimeSpan timeSpan(0,m_nStopLength,0,0);
				dtStop = dtStart + timeSpan;
			}
			break;
		case TYPE_DAY:
			{
				COleDateTimeSpan timeSpan(m_nStopLength,0,0,0);
				dtStop = dtStart + timeSpan;
			}
			break;
		case TYPE_WEEK:
			{
				COleDateTimeSpan timeSpan(m_nStopLength*7,0,0,0);
				dtStop = dtStart + timeSpan;
			}
			break;
		case TYPE_MONTH:
			{
				int nMonth = dtStart.GetMonth() + m_nStopLength;
				int nY = nMonth/12 ;
				int nM = nMonth%12;

				dtStop.SetDateTime(dtStart.GetYear()+nY,nM,dtStart.GetDay(),dtStart.GetHour(),dtStart.GetMinute(),dtStart.GetSecond());

			}
			break;
		case TYPE_YEAR:
			dtStop.SetDateTime(dtStart.GetYear()+m_nStopLength,dtStart.GetMonth(),dtStart.GetDay(),dtStart.GetHour(),dtStart.GetMinute(),dtStart.GetSecond());
			break;
		}

	}
	else
		dtStop = dtStart;

	return dtStop >= dtStart;

//	return bLegal;

}

BOOL  CCAMMScheduleEditorDlg::GetStopDateTime(COleDateTime& dtStop)
{
	CString txtStr;
	TCHAR txt[MAX_LENGTH];
	CScheduleTimeInfo stiStartTime;
	CScheduleTimeInfo stiStopTime;
	COleDateTime dtStart;
	//得到起始时间，解析后放到stiStartTime中
	if((m_CbStartTime.GetStyle() & WS_VISIBLE))
	{
		memset(txt,0x00,MAX_LENGTH);
		m_CbStartTime.GetWindowText(txt,MAX_LENGTH);
		CString txtStr=txt;
		if (!stiStartTime.ParseAMPMTimeString(txtStr))
			return FALSE;
	}
	else
		stiStartTime.SetData(9,0,0);
	//得到起始日期，放到dtStart中
	if((m_cbStartDate.GetStyle() & WS_VISIBLE))
	{
		int nH,nMin;
		stiStartTime.Get24HourTime(nH,nMin);
		dtStart.SetDateTime(m_StartDate.GetYear(),m_StartDate.GetMonth(),m_StartDate.GetDay(),nH,nMin,0);
	}
	else
		return FALSE;

	//得到结束时间，放到stiStopTime中
	if((m_CbStopTime.GetStyle() & WS_VISIBLE) ||(m_cbStopDate.GetStyle() & WS_VISIBLE) )
	{
		if((m_CbStopTime.GetStyle() & WS_VISIBLE))
		{
			memset(txt,0x00,MAX_LENGTH);
			m_CbStopTime.GetWindowText(txt,MAX_LENGTH);
			txtStr=txt;
			if (!stiStopTime.ParseAMPMTimeString(txtStr)) 
				return FALSE;
		}
		else
			stiStopTime = stiStartTime;
		//得到结束的日期
		if((m_cbStopDate.GetStyle() & WS_VISIBLE))
		{
			int nH,nMin;
			stiStopTime.Get24HourTime(nH,nMin);
			dtStop.SetDateTime(m_StopDate.GetYear(),m_StopDate.GetMonth(),m_StopDate.GetDay(),nH,nMin,0);
		}
		else
		{
			int nH,nMin;
			stiStopTime.Get24HourTime(nH,nMin);
			dtStop.SetDateTime(m_StartDate.GetYear(),m_StartDate.GetMonth(),m_StartDate.GetDay(),nH,nMin,0);
		}


	}
	//9u9没有用到结束的类型与结束时间编辑框
	else if((m_cbStopTimeType.GetStyle() & WS_VISIBLE)  && (m_edtStopTime.GetStyle() & WS_VISIBLE))
	{
		COleDateTime Interval;	
		TimeLengthType type =(TimeLengthType)(m_cbStopTimeType.GetItemData( m_cbStopTimeType.GetCurSel()));
		switch(type)
		{
		case TYPE_SEC:
			{
				COleDateTimeSpan timeSpan(0,0,0,m_nStopLength);
				dtStop = dtStart + timeSpan;
			}
			break;
		case TYPE_MIN:
			{
				COleDateTimeSpan timeSpan(0,0,m_nStopLength,0);
				dtStop = dtStart + timeSpan;
			}
			break;
		case TYPE_HOUR:
			{
				COleDateTimeSpan timeSpan(0,m_nStopLength,0,0);
				dtStop = dtStart + timeSpan;
			}
			break;
		case TYPE_DAY:
			{
				COleDateTimeSpan timeSpan(m_nStopLength,0,0,0);
				dtStop = dtStart + timeSpan;
			}
			break;
		case TYPE_WEEK:
			{
				COleDateTimeSpan timeSpan(m_nStopLength*7,0,0,0);
				dtStop = dtStart + timeSpan;
			}
			break;
		case TYPE_MONTH:
			{
				int nMonth = dtStart.GetMonth() + m_nStopLength;
				int nY = nMonth/12 ;
				int nM = nMonth%12;

				dtStop.SetDateTime(dtStart.GetYear()+nY,nM,dtStart.GetDay(),dtStart.GetHour(),dtStart.GetMinute(),dtStart.GetSecond());

			}
			break;
		case TYPE_YEAR:
			dtStop.SetDateTime(dtStart.GetYear()+m_nStopLength,dtStart.GetMonth(),dtStart.GetDay(),dtStart.GetHour(),dtStart.GetMinute(),dtStart.GetSecond());
			break;
		}

	}
	else	//如果没有设置，结束时间等于起始时间
		dtStop = dtStart;
	return TRUE;
}
BOOL CCAMMScheduleEditorDlg::SetAlarmIntervalValue(COleDateTime dtStartDateTime,COleDateTime dtAlarmDateTime)
{
	if(!((m_cbAlarmTimeType.GetStyle() & WS_VISIBLE)  && (m_edtAlarmTime.GetStyle() & WS_VISIBLE)))
		return FALSE;
	if(dtStartDateTime <dtAlarmDateTime)
		return FALSE;
	TimeLengthType nType;
	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000080) //alarmTime
	{
		bool bSupportHour = false;
		for(int i = 0 ;i<m_cbAlarmTimeType.GetCount();i++)
		{
			TimeLengthType type =(TimeLengthType)(m_cbAlarmTimeType.GetItemData(i));
			if(type == TYPE_HOUR)
			{
				bSupportHour = true;
				break;
			}
		}
		if(bSupportHour == false)
		{
			GetIntervalTypenValue_Day(dtAlarmDateTime,dtStartDateTime,nType,m_AlarmLength,
				m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Day,m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Week);
		}
		else 
			GetIntervalTypenValue(dtAlarmDateTime,dtStartDateTime,nType,m_AlarmLength,
			m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Day,m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Week);
	}
	else 
		GetIntervalTypenValue(dtAlarmDateTime,dtStartDateTime,nType,m_AlarmLength,
		m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Day,m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Week);

	for(int i = 0 ;i<m_cbAlarmTimeType.GetCount();i++)
	{
		TimeLengthType type =(TimeLengthType)(m_cbAlarmTimeType.GetItemData(i));
		if(type == nType)
		{
			m_cbAlarmTimeType.SetCurSel(i);

/////////
			int nMax = m_nMaxAlarmLength;
			switch(type)
			{
				case TYPE_SEC:
					nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Sec;
					break;
				case TYPE_MIN:
					nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Min;
					break;
				case TYPE_HOUR:
					nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Hour;
					break;
				case TYPE_DAY:
					nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Day;
					break;
				case TYPE_WEEK:
					nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Week;
					break;
				case TYPE_MONTH:
					nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Month;
					break;
				case TYPE_YEAR:
					nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Year;
					break;
			}
/////////
			if(m_AlarmLength > nMax)
				m_AlarmLength = nMax;
			break;
		}
	}
	if(i>= m_cbAlarmTimeType.GetCount() || m_AlarmLength == 0)
	{
		m_cbAlarmTimeType.SetCurSel(0);
		m_AlarmLength = 0;
	}
	xUpdateData(FALSE);
	return TRUE;
}
BOOL CCAMMScheduleEditorDlg::GetIntervalTypenValue(COleDateTime value1,COleDateTime value2 , TimeLengthType& nType,int& dwIntervalLength,
												   DWORD MaxDayValue,DWORD MaxWeekValue)
{
	if(value2 < value1)
		return FALSE;

	COleDateTime dtSpan = value2 - value1;
	int nS = dtSpan.GetSecond() ;
	int nMin = dtSpan.GetMinute() ;
	int nH = dtSpan.GetHour() ;
	int nD = dtSpan.GetDay() ;
	int nM = dtSpan.GetMonth() ;
	int nY = dtSpan.GetYear() ;

	int nDay =(int) dtSpan.m_dt ;
	if(nS>0)
	{
		nType = TYPE_SEC;
		dwIntervalLength =(nDay*24*60+ nMin)*60+nS;
	}
	else if(nMin > 0)
	{
		DWORD dwMin = ((nDay*24) + nH)*60+ nMin;
		nType = TYPE_MIN;
		dwIntervalLength = dwMin;

	}
	else if(nH>0)
	{
		DWORD dwH = nDay*24+ nH;
		nType = TYPE_HOUR;
		dwIntervalLength = dwH;
	}
	else if(nD > 0)
	{
		if((nDay %7) == 0)
		{
			dwIntervalLength = nDay/7;
			if(dwIntervalLength > MaxWeekValue)
			{
				if(nDay<=MaxDayValue)
				{
					dwIntervalLength = nDay;
					nType = TYPE_DAY;
				}
				else
				{
					if(MaxWeekValue*7 > MaxDayValue)
					{
						dwIntervalLength = nDay/7;
						nType = TYPE_WEEK;
					}
					else
					{
						dwIntervalLength = MaxDayValue;
						nType = TYPE_DAY;
					}
				}
			}
			else
				nType = TYPE_WEEK;
		}
		else if(nDay<=MaxDayValue)
		{
			dwIntervalLength = nDay;
			nType = TYPE_DAY;
		}
		else
		{
			dwIntervalLength = MaxDayValue;
			nType = TYPE_DAY;
		}
	}
	else if(nM > 0)
	{
		dwIntervalLength = nM+12*nY;
		nType = TYPE_MONTH;
	}
	else if(nY > 0)
	{
		dwIntervalLength = nY;
		nType = TYPE_YEAR;
	}

	return TRUE;
}
BOOL CCAMMScheduleEditorDlg::GetIntervalTypenValue_Day(COleDateTime value1,COleDateTime value2 , TimeLengthType& nType,int& dwIntervalLength,
													   DWORD MaxDayValue,DWORD MaxWeekValue)
{
	if(value2 < value1)
		return FALSE;
	COleDateTime dt1,dt2 ;
	dt1.SetDateTime(value1.GetYear(),value1.GetMonth(),value1.GetDay(),0,0,0);
	dt2.SetDateTime(value2.GetYear(),value2.GetMonth(),value2.GetDay(),0,0,0);
	COleDateTime dtSpan = dt2 - dt1;
	int nD = dtSpan.GetDay() ;
	int nM = dtSpan.GetMonth() ;
	int nY = dtSpan.GetYear() ;

	int nDay =(int) dtSpan.m_dt ;

	if(nD > 0)
	{
		if((nDay %7) == 0)
		{
			dwIntervalLength = nDay/7;
			if(dwIntervalLength > MaxWeekValue)
			{
				if(nDay<=MaxDayValue)
				{
					dwIntervalLength = nDay;
					nType = TYPE_DAY;
				}
				else
				{
					if(MaxWeekValue*7 > MaxDayValue)
					{
						dwIntervalLength = nDay/7;
						nType = TYPE_WEEK;
					}
					else
					{
						dwIntervalLength = MaxDayValue;
						nType = TYPE_DAY;
					}
				}
			}
			else
				nType = TYPE_WEEK;
		}
		else if(nDay<=MaxDayValue)
		{
			dwIntervalLength = nDay;
			nType = TYPE_DAY;
		}
		else
		{
			dwIntervalLength = MaxDayValue;
			nType = TYPE_DAY;
		}
	}
	else if(nM > 0)
	{
		dwIntervalLength = nM+12*nY;
		nType = TYPE_MONTH;
	}
	else if(nY > 0)
	{
		dwIntervalLength = nY;
		nType = TYPE_YEAR;
	}

	return TRUE;
}
BOOL CCAMMScheduleEditorDlg::SetStopIntervalValue(COleDateTime dtStart,COleDateTime dtStop)
{
	if(!((m_cbStopTimeType.GetStyle() & WS_VISIBLE)  && (m_edtStopTime.GetStyle() & WS_VISIBLE)))
		return FALSE;

/*
	CString txtStr;
	TCHAR txt[MAX_LENGTH];
	CScheduleTimeInfo stiStartTime;
	CScheduleTimeInfo stiStopTime;
	COleDateTime dtStart,dtStop;

	if(m_CbStartTime.IsWindowVisible())
	{
		memset(txt,0x00,MAX_LENGTH);
		m_CbStartTime.GetWindowText(txt,MAX_LENGTH);
		CString txtStr=txt;
		if (!stiStartTime.ParseAMPMTimeString(txtStr))
			return FALSE;
	}
	else
		stiStartTime.SetData(9,0,0);

	if(m_cbStartDate.IsWindowVisible())
	{
		int nH,nMin;
		stiStartTime.Get24HourTime(nH,nMin);
		dtStart.SetDateTime(m_StartDate.GetYear(),m_StartDate.GetMonth(),m_StartDate.GetDay(),nH,nMin,0);
	}
	else
		return FALSE;
	//GetStopTime

	if(m_CbStopTime.IsWindowEnabled())
	{
		memset(txt,0x00,MAX_LENGTH);
		m_CbStopTime.GetWindowText(txt,MAX_LENGTH);
		txtStr=txt;
		if (!stiStopTime.ParseAMPMTimeString(txtStr)) 
			return FALSE;
	}
	else
		stiStopTime = stiStartTime;

	if(m_cbStopDate.IsWindowEnabled())
	{
		int nH,nMin;
		stiStopTime.Get24HourTime(nH,nMin);
		dtStop.SetDateTime(m_StopDate.GetYear(),m_StopDate.GetMonth(),m_StopDate.GetDay(),nH,nMin,0);
	}
	else
	{
		int nH,nMin;
		stiStopTime.Get24HourTime(nH,nMin);
		dtStop.SetDateTime(m_StartDate.GetYear(),m_StartDate.GetMonth(),m_StartDate.GetDay(),nH,nMin,0);
	}*/
	if(dtStop < dtStart)
		return FALSE;

/*	COleDateTime dtSpan = dtStop - dtStart;
	int nS = dtSpan.GetSecond() ;
	int nMin = dtSpan.GetMinute() ;
	int nH = dtSpan.GetHour() ;
	int nD = dtSpan.GetDay() ;
	int nM = dtSpan.GetMonth() ;
	int nY = dtSpan.GetYear() ;

	TimeLengthType nType;
	int nDay = dtSpan.m_dt ;
	if(nS>0)
	{
		nType = TYPE_SEC;
		m_nStopLength =(nDay*24*60+ nMin)*60+nS;
	}
	else if(nMin > 0)
	{
		DWORD dwMin = nDay*24*60+ nMin;
		nType = TYPE_MIN;
		m_nStopLength = dwMin;

	}
	else if(nH>0)
	{
		DWORD dwH = nDay*24+ nH;
		nType = TYPE_HOUR;
		m_nStopLength = dwH;
	}
	else if(nD > 0)
	{
		if((nDay %7) == 0)
		{
			m_nStopLength = nDay/7;
			nType = TYPE_WEEK;
		}
		else if(nDay<=m_nMaxStopLength)
		{
			m_nStopLength = nDay;
			nType = TYPE_DAY;
		}
		else
		{
			m_nStopLength = m_nMaxStopLength;
			nType = TYPE_DAY;
		}
	}
	else if(nM > 0)
	{
		m_nStopLength = nM+12*nY;
		nType = TYPE_MONTH;
	}
	else if(nY > 0)
	{
		m_nStopLength = nY;
		nType = TYPE_YEAR;
	}
*/
	TimeLengthType nType;
	GetIntervalTypenValue(dtStart,dtStop ,nType,m_nStopLength,m_nMaxStopLength,m_nMaxStopLength);
	for(int i = 0 ;i<m_cbStopTimeType.GetCount();i++)
	{
		TimeLengthType type =(TimeLengthType)(m_cbStopTimeType.GetItemData(i));
		if(type == nType)
		{
			m_cbStopTimeType.SetCurSel(i);
			if(m_nStopLength > m_nMaxStopLength)
				m_nStopLength = m_nMaxStopLength;
			break;
		}
	}
	if(i>= m_cbStopTimeType.GetCount())
	{
		m_cbStopTimeType.SetCurSel(0);
		m_nStopLength = 0;
	}
	xUpdateData(FALSE);
	return TRUE;

}
/*
BOOL CCAMMScheduleEditorDlg::UpdateStopTime(DWORD dwTimeIntervalMin, int nStartTimeIndex)
{
	int nIndex = dwTimeIntervalMin/30;
	xUpdateStopTimeCb(nStartTimeIndex);
//	if(m_CbStopTime.IsWindowVisible() || m_cbStopDate.IsWindowVisible())
	{
		int nStopIndex = 0;
		if(m_CbStopTime.IsWindowEnabled())
		{
			nStopIndex = max(0,nIndex);
		}
		else
			nStopIndex = 0;
		m_CbStopTime.SetCurSel(nStopIndex);
	}

	if(m_CbStopTime.IsWindowVisible() == FALSE && m_cbStopDate.IsWindowVisible()== FALSE)
	{
		COleDateTime dtStop;
		if(GetStopDateTime(dtStop))
		{
			m_StopDate =dtStop;
			CString str,strStop;
			int nH = dtStop.GetHour();
			int nM = dtStop.GetMinute();
			if(nH >=12 )
			{
				str =LoadStringFromFile(_T("public"),"PM");
				if(nH >12)	nH -=12;
			}
			else 
			{
				str =LoadStringFromFile(_T("public"),"AM");
				if(nH == 0) nH =12;
			}
			strStop.Format("%s %02d:%02d",str,nH,nM);
			m_CbStopTime.SetWindowText(strStop);
		}
	}
	return TRUE;

}
*/
BOOL  CCAMMScheduleEditorDlg::GetAlarmDateTime(COleDateTime dtStartDate24Hur ,COleDateTime& dtAlarm,BOOL& bTimeOnly)
{
	bTimeOnly = FALSE;	
	//闹铃的提醒模式，例如：提前5分钟
	if((m_cbAlarmLeadTime.GetStyle() & WS_VISIBLE) && GetAlarmLeadTime() != 65535)
	{
		COleDateTimeSpan dtAlarmLeadTimeSpan;
		//GetAlarmleadTime 得到提前的分钟数 为了取值方便 在init时将item与data赋予了同样的值
		dtAlarmLeadTimeSpan.SetDateTimeSpan(0,0,GetAlarmLeadTime(),0);	
		dtAlarm = dtStartDate24Hur - dtAlarmLeadTimeSpan;	//计算出闹铃的时间 注意dtAlarm是引用
	}
	//下面是允许手工输入闹铃时间的情况 暂时不用
	else if((m_cbAlarmTimeType.GetStyle() & WS_VISIBLE)  && (m_edtAlarmTime.GetStyle() & WS_VISIBLE))
	{
		COleDateTime Interval;	
		TimeLengthType type =(TimeLengthType)(m_cbAlarmTimeType.GetItemData( m_cbAlarmTimeType.GetCurSel()));
		switch(type)
		{
		case TYPE_SEC:
			{
				COleDateTimeSpan timeSpan(0,0,0,m_AlarmLength);
				dtAlarm = dtStartDate24Hur - timeSpan;
			}
			break;
		case TYPE_MIN:
			{
				COleDateTimeSpan timeSpan(0,0,m_AlarmLength,0);
				dtAlarm = dtStartDate24Hur - timeSpan;
			}
			break;
		case TYPE_HOUR:
			{
				COleDateTimeSpan timeSpan(0,m_AlarmLength,0,0);
				dtAlarm = dtStartDate24Hur - timeSpan;
			}
			break;
		case TYPE_DAY:
			{
				COleDateTimeSpan timeSpan(m_AlarmLength,0,0,0);
				dtAlarm = dtStartDate24Hur - timeSpan;
			}
			break;
		case TYPE_WEEK:
			{
				COleDateTimeSpan timeSpan(m_AlarmLength*7,0,0,0);
				dtAlarm = dtStartDate24Hur - timeSpan;
			}
			break;
		case TYPE_MONTH:
			{
				int nMonth = dtStartDate24Hur.GetMonth()+dtStartDate24Hur.GetYear()*12 - m_AlarmLength;
				int nY = nMonth/12 ;
				int nM = nMonth%12;

				dtAlarm.SetDateTime(nY,nM,dtStartDate24Hur.GetDay(),dtStartDate24Hur.GetHour(),dtStartDate24Hur.GetMinute(),dtStartDate24Hur.GetSecond());

			}

			break;
		case TYPE_YEAR:
			dtAlarm.SetDateTime(dtStartDate24Hur.GetYear()-m_AlarmLength,dtStartDate24Hur.GetMonth(),dtStartDate24Hur.GetDay(),dtStartDate24Hur.GetHour(),dtStartDate24Hur.GetMinute(),dtStartDate24Hur.GetSecond());
			break;
		}

		if(type!= TYPE_SEC && type!= TYPE_MIN && type!= TYPE_HOUR)
		{
			if((m_cbAlarmTime.GetStyle() & WS_VISIBLE) && m_cbAlarmTime.IsWindowEnabled())
			{
				int nY,nM,nD,nH,nMin,ns;
				nH =m_AlarmTime.GetHour() ;
				nMin =m_AlarmTime.GetMinute(); 
				ns =m_AlarmTime.GetSecond();
				nY = dtAlarm.GetYear();
				nM = dtAlarm.GetMonth();
				nD = dtAlarm.GetDay();
				dtAlarm.SetDateTime(nY,nM,nD,nH,nMin, ns);
			}
		}
	}
	else	//暂时不用
	{
		int nY,nM,nD,nH,nMin,ns;
		if((m_cbAlarmday.GetStyle() & WS_VISIBLE))
		{
			nY = m_Alarmday.GetYear();
			nM = m_Alarmday.GetMonth();
			nD = m_Alarmday.GetDay();
		}
		else
		{
			nY = m_StartDate.GetYear();
			nM = m_StartDate.GetMonth();
			nD = m_StartDate.GetDay();

			if((m_cbAlarmLeadTime.GetStyle() & WS_VISIBLE) == FALSE)
			{
				bTimeOnly = TRUE;
			}

		}

		if((m_cbAlarmTime.GetStyle() & WS_VISIBLE))
		{
			nH =m_AlarmTime.GetHour() ;
			nMin =m_AlarmTime.GetMinute(); 
			ns =m_AlarmTime.GetSecond();
		}
		else
		{
			TCHAR txt[MAX_LENGTH];
			memset(txt,0x00,MAX_LENGTH);
			m_CbStartTime.GetWindowText(txt,MAX_LENGTH);

			CString txtStr=txt;
			CScheduleTimeInfo startTimeInfo;
			startTimeInfo.ParseAMPMTimeString(txtStr);

			startTimeInfo.Get24HourTime(nH,nMin);
			ns =0;
		}
		dtAlarm.SetDateTime(nY,nM,nD,nH,nMin, ns);
	}
	if(dtAlarm.GetStatus() == COleDateTime::invalid)
		dtAlarm = dtStartDate24Hur;

	return ((m_cbAlarmLeadTime.GetStyle() & WS_VISIBLE) || 
			(m_cbAlarmday.GetStyle() & WS_VISIBLE) ||
			(m_cbAlarmTime.GetStyle() & WS_VISIBLE) ||  
			((m_cbAlarmTimeType.GetStyle() & WS_VISIBLE)  && (m_edtAlarmTime.GetStyle() & WS_VISIBLE)));
}

void CCAMMScheduleEditorDlg::OnKillfocusSchDataAlarmtimeedit() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(!((m_cbAlarmTimeType.GetStyle() & WS_VISIBLE)  && (m_edtAlarmTime.GetStyle() & WS_VISIBLE)))
		return ;

	TimeLengthType type =(TimeLengthType)(m_cbAlarmTimeType.GetItemData( m_cbAlarmTimeType.GetCurSel()));
	int nMax = m_nMaxAlarmLength;
	switch(type)
	{
		case TYPE_SEC:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Sec;
			break;
		case TYPE_MIN:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Min;
			break;
		case TYPE_HOUR:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Hour;
			break;
		case TYPE_DAY:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Day;
			break;
		case TYPE_WEEK:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Week;
			break;
		case TYPE_MONTH:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Month;
			break;
		case TYPE_YEAR:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Year;
			break;
	}
	if(m_AlarmLength > nMax)
	{
		CString strMsg; 
		CString str = LoadStringFromFile(_T("message"),_T("IDS_CAL_MSG_ALARM_ERROR"));
		strMsg.Format(str,nMax);
		CalendarStringMessageBox(m_hWnd,strMsg);
		TCHAR szText[MAX_PATH];
		_itot(nMax,szText,10);
		m_edtAlarmTime.SetWindowText(szText);

	}
}

void CCAMMScheduleEditorDlg::OnKillfocusSchDataStoptimeedit() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	
}

void CCAMMScheduleEditorDlg::SetRepeatType(int nType,int nDayofWeekMask)
{
	if(nType == TWOWEEKLY && (m_edRepeatInterval.GetStyle() & WS_VISIBLE))
	{
		//nType = WEEKLY;//080612LIBAOLIU
	}
	else if(nType == WEEKLY && nDayofWeekMask == 62)
	{
		//if((m_dwRepeatTypeFlag>>8) &1) //weekdays
			//nType = WEEKDAYS;//080612LIBAOLIU
	}
	else if(nType == WEEKLY && nDayofWeekMask == 65)
	{
		//if((m_dwRepeatTypeFlag>>7) &1) //weekend
			//nType = WEEKEND;//080612LIBAOLIU
	}
	//增加对自定义的判断 kerm add for 9a9u 1.05
	else if(nType == WEEKLY && nDayofWeekMask != 0)	//只有自定义不为零
	{
		//nType = WEEKDAYS;	//用来指示combox的代号//080612LIBAOLIU
	}

	int nCount = m_cbRepeatType.GetCount();
	for(int i = 0 ; i<nCount ;i++)
	{
		if(nType == m_cbRepeatType.GetItemData(i))
		{
// 			CString st;
// 			st.Format(L"看看i %d", nType);
// 			AfxMessageBox(st);

			m_cbRepeatType.SetCurSel(i);
			if(3 == i)
			{
				m_chWeek1.EnableWindow();
				m_chWeek2.EnableWindow();
				m_chWeek3.EnableWindow();
				m_chWeek4.EnableWindow();
				m_chWeek5.EnableWindow();
				m_chWeek6.EnableWindow();
				m_chWeek7.EnableWindow();
				if(m_ScheduleInfo.nDayofWeekMask & 1 ) m_chWeek1.SetCheck(true);
				if(m_ScheduleInfo.nDayofWeekMask & 2 ) m_chWeek2.SetCheck(true);
				if(m_ScheduleInfo.nDayofWeekMask & 4 ) m_chWeek3.SetCheck(true);
				if(m_ScheduleInfo.nDayofWeekMask & 8 ) m_chWeek4.SetCheck(true);
				if(m_ScheduleInfo.nDayofWeekMask & 16 ) m_chWeek5.SetCheck(true);
				if(m_ScheduleInfo.nDayofWeekMask & 32 ) m_chWeek6.SetCheck(true);
				if(m_ScheduleInfo.nDayofWeekMask & 64 ) m_chWeek7.SetCheck(true);

	
			}
			break;
		}
	}
	if(i >= nCount)
		m_cbRepeatType.SetCurSel(0);
}

void CCAMMScheduleEditorDlg::UpdateDateofWeekCtlStates()
{

	int nRepeatType = m_cbRepeatType.GetItemData(m_cbRepeatType.GetCurSel());
	int nDayofWeekMask = m_ScheduleInfo.nDayofWeekMask;

	CCheckEx2	*pCheck[7] = {&m_chWeek1,&m_chWeek2,&m_chWeek3,&m_chWeek4,&m_chWeek5,&m_chWeek6,&m_chWeek7};
	//在99u中，只有选择days才激活7天， 修改最后一个条件 kerm 12.28
	if((m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00002000 )&& 
	 m_chkRepeat.GetCheck() && (nRepeatType == WEEKDAYS)) //repeatdayofweekmask
	{ 
		int i;
		//激活7天，但是只有周一到周五选中，直接修改下面代码 kerm 12.28
		for(i = 0; i< 7 ; i++)
		{
			pCheck[i]->EnableWindow(true);
			pCheck[i]->SetCheck(true);
			if(i == 0 || i == 6)
				pCheck[i]->SetCheck(false);
		}
		//下面根据开始日期设置选定状态，在99u中不这样做 暂时不用  
		//9a9u中需要根据dayofweek的值来确定选中的状态
/*									kerm change for 9a9u 1.05	
		int nWeek = m_StartDate.GetDayOfWeek()-1;
		pCheck[nWeek]->SetCheck(true);
		pCheck[nWeek]->EnableWindow(false);
		if((m_nDayofWeekMask >> nWeek) &  1)
		{
			for(i = 0; i< 7 ; i++)
			{
				if((m_nDayofWeekMask >> i) &  1) pCheck[i]->SetCheck(true);
			}
		}*/
		//根据dayofweek来确定选定的状态  kerm add for 9a9u 1.05
		if(nDayofWeekMask != 0)
		{
			//先将选中的取消
			for(i = 0; i< 7 ; i++)
			{
				pCheck[i]->EnableWindow(true);
				pCheck[i]->SetCheck(false);
			}
			m_chWeek1.EnableWindow();
			m_chWeek2.EnableWindow();
			m_chWeek3.EnableWindow();
			m_chWeek4.EnableWindow();
			m_chWeek5.EnableWindow();
			m_chWeek6.EnableWindow();
			m_chWeek7.EnableWindow();
			if(nDayofWeekMask & 1 ) m_chWeek1.SetCheck(true);
			if(nDayofWeekMask & 2 ) m_chWeek2.SetCheck(true);
			if(nDayofWeekMask & 4 ) m_chWeek3.SetCheck(true);
			if(nDayofWeekMask & 8 ) m_chWeek4.SetCheck(true);
			if(nDayofWeekMask & 16 ) m_chWeek5.SetCheck(true);
			if(nDayofWeekMask & 32 ) m_chWeek6.SetCheck(true);
			if(nDayofWeekMask & 64 ) m_chWeek7.SetCheck(true);
		}


	}
	else
	{
		for(int i = 0; i< 7 ; i++)
		{
			pCheck[i]->EnableWindow(false);
			pCheck[i]->SetCheck(false);
		}
	}

	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].bDisableEndDateWhenRepeat)
	{
		if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000010) //StopDate
		{
			m_cbStopDate.EnableWindow(!m_chkRepeat.GetCheck());
		}
	}
}

void CCAMMScheduleEditorDlg::OnChangeEdRepeateinter() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

}

void CCAMMScheduleEditorDlg::OnKillfocusEdRepeateinter() 
{
	// TODO: Add your control notification handler code here
	int lower,upper;
	m_spRepeatinterval.GetRange(lower,upper);
	TCHAR szNum[MAX_PATH];
	m_edRepeatInterval.GetWindowText(szNum,MAX_PATH);
	int n = _ttoi(szNum);
	if(n < lower)
	{
		_itot(lower,szNum,10);
		m_edRepeatInterval.SetWindowText(szNum);
	}
	else if(n>upper)
	{
		_itot(upper,szNum,10);
		m_edRepeatInterval.SetWindowText(szNum);
	}
	
}
BOOL CCAMMScheduleEditorDlg::IsEnabledAlarmTime() 
{
	bool bSupport = false;
	if(m_ArrayofSupportNoteType[m_cbIndex_NoteType].dwSupportFlag & 0x00000080) //alarmTime
	{
		if(!((m_cbAlarmTimeType.GetStyle() & WS_VISIBLE)  && (m_edtAlarmTime.GetStyle() & WS_VISIBLE)))
			return true;
	
		bool bSupportHour = false;
		for(int i = 0 ;i<m_cbAlarmTimeType.GetCount();i++)
		{
			TimeLengthType type =(TimeLengthType)(m_cbAlarmTimeType.GetItemData(i));
			if(type == TYPE_HOUR)
			{
				bSupportHour = true;
				break;
			}
		}
		if(bSupportHour == false)
		{
			return true;
		}
		else 
			return false;
	}
	else 
		bSupport = false;
	return bSupport;

}

void CCAMMScheduleEditorDlg::OnSelchangeSchDataAlarmtimetype() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(!((m_cbAlarmTimeType.GetStyle() & WS_VISIBLE)  && (m_edtAlarmTime.GetStyle() & WS_VISIBLE)))
		return ;

	TimeLengthType type =(TimeLengthType)(m_cbAlarmTimeType.GetItemData( m_cbAlarmTimeType.GetCurSel()));
	int nMax = m_nMaxAlarmLength;
	switch(type)
	{
		case TYPE_SEC:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Sec;
			break;
		case TYPE_MIN:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Min;
			break;
		case TYPE_HOUR:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Hour;
			break;
		case TYPE_DAY:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Day;
			break;
		case TYPE_WEEK:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Week;
			break;
		case TYPE_MONTH:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Month;
			break;
		case TYPE_YEAR:
			nMax =m_ArrayofSupportNoteType[m_cbIndex_NoteType].nAlarmTimeIntervalMax_Year;
			break;
	}
	if(m_AlarmLength > nMax)
	{
		TCHAR szText[MAX_PATH];
		_itot(nMax,szText,10);
		m_edtAlarmTime.SetWindowText(szText);
	}
}

void CCAMMScheduleEditorDlg::SetViewOnlyMode(bool bViewOnly)
{
	m_bViewOnly = bViewOnly;

}

void CCAMMScheduleEditorDlg::DisableAllControl()
{
	m_spRepeatinterval.EnableWindow(false);
	m_edRepeatInterval.EnableWindow(false);
	m_scRepeatON.EnableWindow(false);
	m_scRepeatinterval.EnableWindow(false);
	m_chWeek7.EnableWindow(false);
	m_chWeek6.EnableWindow(false);
	m_chWeek5.EnableWindow(false);
	m_chWeek4.EnableWindow(false);
	m_chWeek3.EnableWindow(false);
	m_chWeek2.EnableWindow(false);
	m_chWeek1.EnableWindow(false);
	m_edDescription.EnableWindow(false);
	m_stDescription.EnableWindow(false);
	m_chkRepeatEndDate.EnableWindow(false);
	m_stRepeatEndData.EnableWindow(false);
	m_cbAlarmTimeType.EnableWindow(false);
	m_edtAlarmTime.EnableWindow(false);
	m_cbStopTimeType.EnableWindow(false);
	m_edtStopTime.EnableWindow(false);
	m_cbAlarmTime.EnableWindow(false);
	m_cbAlarmday.EnableWindow(false);
	m_cbStartDate.EnableWindow(false);
	m_stLtType.EnableWindow(false);
	m_stLtTitle.EnableWindow(false);
	m_stAlarmPhoneNo.EnableWindow(false);
	m_stStartTime.EnableWindow(false);
	m_stStopTime.EnableWindow(false);
	m_chkAlarm.EnableWindow(false);
	m_chkRepeat.EnableWindow(false);
	m_ctrlOk.ShowWindow(SW_HIDE);
	m_ckTone.EnableWindow(false);
	m_cbNoteType.EnableWindow(false);
	m_stLocation.EnableWindow(false);
	m_cbStopDate.EnableWindow(false);
	m_etLocation.EnableWindow(false);
	m_etAlarmPhoneNo.EnableWindow(false);
	m_cbAlarmLeadTime.EnableWindow(false);
	m_cbRepeatWhichWeek.EnableWindow(false);
	m_cbWeekDay.EnableWindow(false);
	m_cbRepeatType.EnableWindow(false);
	m_CbStopTime.EnableWindow(false);
	m_CbStartTime.EnableWindow(false);
	GetDlgItem(IDC_SCH_DATA_REPEAT_WHICHDATE)->EnableWindow(false);;
	//只读模式下禁用优先级   kerm add for 9a9u 
	m_cbPriority.EnableWindow(false);
	m_stPriority.EnableWindow(false);

	TCHAR szTxt[MAX_PATH];
	al_GetSettingString(_T("public"),_T("IDS_OK"),theApp.m_szRes,szTxt);
	m_ctrlCancel.SetWindowText(szTxt);

	GetDlgItem(IDC_SCH_DATA_TITLE)->EnableWindow(false);
}
LRESULT CCAMMScheduleEditorDlg::OnCheckViewMode(WPARAM, LPARAM)
{
	if(m_bViewOnly)
		DisableAllControl();
	return 1L;
}
