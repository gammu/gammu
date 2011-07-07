; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CDialogBarEx
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "calendar.h"
LastPage=0

ClassCount=14
Class1=CCalendarApp
Class2=CCheckThumbListCtrl
Class3=CLeftView
Class4=CMainFrame
Class5=CMyEdit
Class6=COpenrepeatQuestDlg
Class7=CPrevView
Class8=CProcessDlg
Class9=CSearchDlg
Class10=CUserExitDlg
Class11=CViewModePanel

ResourceCount=15
Resource1=IDD_DLG_PROG (English (U.S.))
Resource2=IDR_TOOLBAR_LEFTHANDSIDE (English (U.S.))
Resource3=IDD_DLGBAR
Resource4=IDD_ABOUTBOX (English (U.S.))
Resource5=IDD_SEARCH (English (U.S.))
Resource6=IDR_TB_FUNC (English (U.S.))
Resource7=IDD_OPENREPEATQUESTDLG (English (U.S.))
Resource8=IDR_MAINFRAME (Chinese (Taiwan))
Resource9=IDD_PVI_5_USEREXIT (English (U.S.))
Resource10=IDR_MENU_LISTRIGHTKEY
Resource11=IDD_DLG_TOPBAR
Class12=CTopView
Resource12=IDD_ORG_SCHEDULE (English (U.S.))
Class13=CTopBarDlg
Resource13=IDR_TB_TOOLBAR (English (U.S.))
Resource14=IDD_ORG_SCHEDULE_EDITOR (English (U.S.))
Class14=CDialogBarEx
Resource15=IDR_TOOLBAR_RIGHTHANDSIDE (English (U.S.))

[CLS:CCalendarApp]
Type=0
BaseClass=CWinApp
HeaderFile=calendar.h
ImplementationFile=calendar.cpp
LastObject=CCalendarApp

[CLS:CCheckThumbListCtrl]
Type=0
BaseClass=CListCtrl
HeaderFile=CheckThumbListCtrl.h
ImplementationFile=CheckThumbListCtrl.cpp

[CLS:CLeftView]
Type=0
BaseClass=CTreeView
HeaderFile=leftview.h
ImplementationFile=leftview.cpp
Filter=C
VirtualFilter=VWC
LastObject=CLeftView

[CLS:CMainFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=mainfrm.h
ImplementationFile=mainfrm.cpp

[CLS:CMyEdit]
Type=0
BaseClass=CEdit
HeaderFile=myedit.h
ImplementationFile=myedit.cpp

[CLS:COpenrepeatQuestDlg]
Type=0
BaseClass=CDialog
HeaderFile=OpenrepeatQuestDlg.h
ImplementationFile=OpenrepeatQuestDlg.cpp

[CLS:CPrevView]
Type=0
BaseClass=CView
HeaderFile=prevview.h
ImplementationFile=prevview.cpp

[CLS:CProcessDlg]
Type=0
BaseClass=CDialog
HeaderFile=processdlg.h
ImplementationFile=processdlg.cpp

[CLS:CSearchDlg]
Type=0
BaseClass=CDialog
HeaderFile=searchdlg.h
ImplementationFile=searchdlg.cpp

[CLS:CUserExitDlg]
Type=0
BaseClass=CDialog
HeaderFile=userexitdlg.h
ImplementationFile=userexitdlg.cpp
LastObject=CUserExitDlg

[CLS:CViewModePanel]
Type=0
BaseClass=CPanel
HeaderFile=viewmodepanel.h
ImplementationFile=viewmodepanel.cpp

[DLG:IDD_OPENREPEATQUESTDLG]
Type=1
Class=COpenrepeatQuestDlg

[DLG:IDD_DLG_PROG]
Type=1
Class=CProcessDlg

[DLG:IDD_PVI_5_USEREXIT]
Type=1
Class=CUserExitDlg

[MNU:IDR_MAINFRAME (Chinese (Taiwan))]
Type=1
Class=?
Command1=ID_APP_EXIT
Command2=ID_EDIT_UNDO
Command3=ID_EDIT_CUT
Command4=ID_EDIT_COPY
Command5=ID_EDIT_PASTE
Command6=ID_VIEW_STATUS_BAR
Command7=ID_APP_ABOUT
CommandCount=7

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=?
ControlCount=4
Control1=IDC_SC_VERSION,static,1342308480
Control2=IDC_SC_COPYRIGHT,static,1342308352
Control3=IDOK,button,1342373899
Control4=IDC_SC_CAMVERSION,static,1342308480

[DLG:IDD_DLG_PROG (English (U.S.))]
Type=1
Class=?
ControlCount=5
Control1=IDC_STOP,button,1342242827
Control2=IDC_PROGRESS_BAR,msctls_progress32,1350565889
Control3=IDC_STATIC_MESSAGE,static,1342308352
Control4=IDCANCEL,button,1073807360
Control5=IDC_ANIMATE,SysAnimate32,1342242820

[DLG:IDD_OPENREPEATQUESTDLG (English (U.S.))]
Type=1
Class=?
ControlCount=5
Control1=IDOK,button,1342242827
Control2=IDCANCEL,button,1342242827
Control3=IDC_STATIC_TITLE,static,1342308352
Control4=IDC_OPENCURRENTRADIO,button,1342177289
Control5=IDC_OPENSERIALRADIO,button,1342177289

[DLG:IDD_SEARCH (English (U.S.))]
Type=1
Class=?
ControlCount=4
Control1=IDCANCEL,button,1342242827
Control2=IDC_BN_FINDNEXT,button,1342242827
Control3=IDC_COMBO_NAME,combobox,1344340226
Control4=IDC_SC_ENTERNAME,static,1342308352

[DLG:IDD_PVI_5_USEREXIT (English (U.S.))]
Type=1
Class=?
ControlCount=5
Control1=IDC_PVI_5_UE_MSG1,static,1342308353
Control2=IDC_PVI_5_UE_SAVE,button,1342242827
Control3=IDC_PVI_5_UE_CANCEL,button,1342242827
Control4=IDC_PVI_5_UE_SAVEAS,button,1342242827
Control5=IDC_PVI_5_UE_DISCARD,button,1342242827

[DLG:IDD_ORG_SCHEDULE (English (U.S.))]
Type=1
Class=?
ControlCount=1
Control1=IDC_ORG_MONTHCALENDAR,SysMonthCal32,1342242816

[DLG:IDD_ORG_SCHEDULE_EDITOR (English (U.S.))]
Type=1
Class=?
ControlCount=49
Control1=IDC_SCH_DATA_NoteType,combobox,1344339971
Control2=IDC_SCH_DATA_TITLE,edit,1350631552
Control3=IDC_SCH_DATA_LOCATION,edit,1216413824
Control4=IDC_SCH_DATA_ALARM_PHONENO,edit,1216413824
Control5=IDC_ED_DESCRIPTION,edit,1484849280
Control6=IDC_SCH_DATA_STARTDATE,SysDateTimePick32,1342242848
Control7=IDC_SCH_DATA_STARTTIME,combobox,1344339970
Control8=IDC_SCH_DATA_STOPDATE,SysDateTimePick32,1342242848
Control9=IDC_SCH_DATA_STOPTIME,combobox,1344339970
Control10=IDC_SCH_DATA_ALARM_INTERVAL,combobox,1344339971
Control11=IDC_SCH_DATA_ALARMDAY,SysDateTimePick32,1476460577
Control12=IDC_SCH_DATA_ALARMTIME,SysDateTimePick32,1476460585
Control13=IDC_SCH_DATA_STOPTIMEEDIT,edit,1216422016
Control14=IDC_SCH_DATA_STOPTIMETYPE,combobox,1478557698
Control15=IDC_SCH_DATA_ALARMTIMEEDIT,edit,1216422016
Control16=IDC_SCH_DATA_ALARMTIMETYPE,combobox,1478557698
Control17=IDC_SCH_DATA_REPEAT_TYPE,combobox,1075904515
Control18=IDC_ED_REPEATEINTER,edit,1484857472
Control19=IDC_CHWEEK1,button,1342242819
Control20=IDC_CHWEEK2,button,1342242819
Control21=IDC_CHWEEK3,button,1342242819
Control22=IDC_CHWEEK4,button,1342242819
Control23=IDC_CHWEEK5,button,1342242819
Control24=IDC_CHWEEK6,button,1342242819
Control25=IDC_CHWEEK7,button,1342242819
Control26=IDC_SCH_DATA_REPEAT_WHICHDATE,SysDateTimePick32,1208025120
Control27=IDOK,button,1342242817
Control28=IDCANCEL,button,1342242816
Control29=IDC_SCH_CHK_ALARM,button,1342242819
Control30=IDC_SCH_CHK_REPEAT,button,1342242819
Control31=IDC_SCH_DATA_REPEAT_WHICHWEEK,combobox,1210122243
Control32=IDC_SCH_DATA_REPEAT_WEEKDAY,combobox,1210122243
Control33=IDC_SCH_CHK_REPEAT_Until,button,1208025091
Control34=IDC_SCH_DATA_MEMO,edit,1218515012
Control35=IDC_SCH_LT_TITLE,static,1342177280
Control36=IDC_SCH_LT_STARTTIME,static,1342177280
Control37=IDC_SCH_LT_STOPTIME,static,1342177280
Control38=IDC_SCH_LT_MEMO,static,1073741824
Control39=IDC_SCH_LT_REMIND_PHONENO,static,1476395020
Control40=IDC_SCH_LT_LOCATION,static,1208090624
Control41=IDC_SCH_LT_TYPE,static,1342308352
Control42=IDC_SCH_CHK_TONE,button,1476460547
Control43=IDC_REPEATENDSTATIC,static,1073872896
Control44=IDC_SC_DESCRIPTION,static,1476526080
Control45=IDC_SC_REPEATON,static,1476526080
Control46=IDC_SC_REPEATE_INTERVAL,static,1342308352
Control47=IDC_SP_REPEATINTER,msctls_updown32,1342177314
Control48=IDC_STC_CAL_PRIORITY,static,1208090624
Control49=IDC_CMB_CAL_PRIORITY,combobox,1478557699

[ACL:IDR_MAINFRAME (Chinese (Taiwan))]
Type=1
Class=?
Command1=ID_EDIT_COPY
Command2=ID_EDIT_PASTE
Command3=ID_EDIT_UNDO
Command4=ID_EDIT_CUT
Command5=ID_NEXT_PANE
Command6=ID_PREV_PANE
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_CUT
Command10=ID_EDIT_UNDO
CommandCount=10

[TB:IDR_TB_FUNC (English (U.S.))]
Type=1
Class=?
Command1=ID_BN_PHONEBOOK
Command2=ID_BN_SCHEDULE
Command3=ID_BN_MEMO
CommandCount=3

[TB:IDR_TB_TOOLBAR (English (U.S.))]
Type=1
Class=?
Command1=ID_BN_NEW
Command2=ID_BN_EDIT
Command3=ID_BN_DELETE
Command4=ID_BN_REFRESH
Command5=ID_BN_EXIT
Command6=ID_BN_ABOUT
Command7=ID_BN_HELP
CommandCount=7

[TB:IDR_TOOLBAR_LEFTHANDSIDE (English (U.S.))]
Type=1
Class=?
Command1=ID_BN_NEW
Command2=ID_BN_EDIT
Command3=ID_BN_DELETE
Command4=ID_BN_REFRESH
CommandCount=4

[TB:IDR_TOOLBAR_RIGHTHANDSIDE (English (U.S.))]
Type=1
Class=?
Command1=ID_BN_EXIT
Command2=ID_BN_ABOUT
Command3=ID_BN_HELP
CommandCount=3

[CLS:CTopView]
Type=0
HeaderFile=TopView.h
ImplementationFile=TopView.cpp
BaseClass=CView
Filter=C
LastObject=CTopView
VirtualFilter=VWC

[DLG:IDD_DLG_TOPBAR]
Type=1
Class=CTopBarDlg
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816

[CLS:CTopBarDlg]
Type=0
HeaderFile=TopBarDlg.h
ImplementationFile=TopBarDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CTopBarDlg

[MNU:IDR_MENU_LISTRIGHTKEY]
Type=1
Class=?
Command1=IDC_MENU_LIST_ADD
Command2=IDC_MENU_LIST_EDIT
Command3=IDC_MENU_LIST_DELETE
Command4=IDC_MENU_LIST_REFLASH
CommandCount=4

[DLG:IDD_DLGBAR]
Type=1
Class=CDialogBarEx
ControlCount=0

[CLS:CDialogBarEx]
Type=0
HeaderFile=DialogBarEx.h
ImplementationFile=DialogBarEx.cpp
BaseClass=CDialog
Filter=D
LastObject=CDialogBarEx

