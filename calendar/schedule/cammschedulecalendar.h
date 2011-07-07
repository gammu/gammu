#if !defined(AFX_CAMMSCHEDULECALENDAR_H__594632D8_681F_467E_B285_F3C98C692213__INCLUDED_)
#define AFX_CAMMSCHEDULECALENDAR_H__594632D8_681F_467E_B285_F3C98C692213__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// cammschedulecalendar.h : header file
//
//#include "..\OrganizerModules.h"
#include "..\resource.h"
#include "..\Organizer\OrganizeDeclaration.h"

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleCalendar dialog

class CCAMMScheduleCalendar : public CDialog
{
// Construction
private:
	HBRUSH m_hBrush;
public:
	void SetPos(CPoint point);
	void GetSelectedDate(COleDateTimeEx &dt);
	void SetCurrentDate(COleDateTimeEx dt);
	CCAMMScheduleCalendar(CWnd* pParent = NULL);   // standard constructor
	~CCAMMScheduleCalendar();

	//Test, Nono_2004_0407
	void CCAMMScheduleCalendar::GetTodayInfo(COleDateTimeEx &dt);

// Dialog Data
	//{{AFX_DATA(CCAMMScheduleCalendar)
	enum { IDD = IDD_ORG_SCHEDULE };
	CMonthCalCtrl	m_MonthCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCAMMScheduleCalendar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCAMMScheduleCalendar)
	afx_msg void OnSelectOrgMonthcalendar(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CPoint m_Point;
	COleDateTimeEx m_CurDate;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMMSCHEDULECALENDAR_H__594632D8_681F_467E_B285_F3C98C692213__INCLUDED_)
