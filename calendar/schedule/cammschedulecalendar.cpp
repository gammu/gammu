// cammschedulecalendar.cpp : implementation file
//

#include "stdafx.h"
//#include "..\camm.h"
#include "cammschedulecalendar.h"
#include "..\Calendar.h"
extern CCalendarApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleCalendar dialog


CCAMMScheduleCalendar::CCAMMScheduleCalendar(CWnd* pParent /*=NULL*/)
	: CDialog(CCAMMScheduleCalendar::IDD, pParent)
{
	m_hBrush=::CreateSolidBrush(RGB(235, 243, 246));
	//{{AFX_DATA_INIT(CCAMMScheduleCalendar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}
CCAMMScheduleCalendar::~CCAMMScheduleCalendar()
{
	DeleteObject(m_hBrush);
}
void CCAMMScheduleCalendar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCAMMScheduleCalendar)
	DDX_Control(pDX, IDC_ORG_MONTHCALENDAR, m_MonthCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCAMMScheduleCalendar, CDialog)
	//{{AFX_MSG_MAP(CCAMMScheduleCalendar)
	ON_NOTIFY(MCN_SELECT, IDC_ORG_MONTHCALENDAR, OnSelectOrgMonthcalendar)
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleCalendar message handlers

void CCAMMScheduleCalendar::OnSelectOrgMonthcalendar(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	LPNMSELCHANGE  lpNMSelChange = (LPNMSELCHANGE) pNMHDR;
	m_CurDate = lpNMSelChange->stSelStart;

	EndDialog(IDOK); 

	*pResult = 0;
}

void CCAMMScheduleCalendar::SetCurrentDate(COleDateTimeEx dt)
{
	m_CurDate = dt;
}

void CCAMMScheduleCalendar::GetSelectedDate(COleDateTimeEx &dt)
{
	dt = m_CurDate;
}

void CCAMMScheduleCalendar::GetTodayInfo(COleDateTimeEx &dt)
{//Test, Nono_2004_0407
	m_MonthCtrl.GetToday(m_CurDate);
	dt = m_CurDate;
}

void CCAMMScheduleCalendar::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	RECT rect,r1;
	GetWindowRect(&rect);
	GetClientRect(&r1);
	CMonthCalCtrl* pWnd = (CMonthCalCtrl*)GetDlgItem(IDC_ORG_MONTHCALENDAR);
	RECT r2;
	pWnd->SizeMinReq();
	pWnd->GetWindowRect(&r2);
	int width = (rect.right-rect.left) - (r1.right-r1.left) + (r2.right-r2.left);
	int height = (rect.bottom-rect.top) - (r1.bottom-r1.top) + (r2.bottom-r2.top);
	//	MoveWindow(m_Point.x,m_Point.y,rect.right-rect.left,rect.bottom-rect.top,FALSE);
	MoveWindow(m_Point.x,m_Point.y,width,height,FALSE);
	pWnd->SetWindowPos(NULL,0,0,0,0,SWP_NOSIZE);
	m_MonthCtrl.SetCurSel(m_CurDate);
	COleDateTime startTime;
	COleDateTime endTime;
	m_MonthCtrl.GetRange(&startTime,&endTime);
	startTime.SetDate(theApp.m_nStartYear,1,1);
	endTime.SetDate(2101,12,31);
	m_MonthCtrl.SetRange(&startTime,&endTime);
	
}

void CCAMMScheduleCalendar::SetPos(CPoint point)
{
	m_Point = point;
}

int CCAMMScheduleCalendar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}


HBRUSH CCAMMScheduleCalendar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	pDC->SetBkColor(RGB(235, 243, 246));
	return m_hBrush;
}