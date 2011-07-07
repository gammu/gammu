; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CAviDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "smsutility.h"
LastPage=0

ClassCount=14
Class1=CAviDlg
Class2=CContactsDlg
Class3=CLeftView
Class4=CMainFrame
Class5=CNewSMS
Class6=COpenSMS
Class7=CPhoneNumberEdit
Class8=CPrevView
Class9=CProcessDlg
Class10=CProgressDlg
Class11=CReportCtrl
Class12=CSkinHeaderCtrl
Class13=CSMSUtility
Class14=CTabSplitterWnd

ResourceCount=10
Resource1=IDD_DLG_NEWSMS (English (U.S.))
Resource2=IDD_DLG_CONTACTS (English (U.S.))
Resource3=IDD_PROCESS_DLG (English (U.S.))
Resource4=IDD_AVI_DLG (English (U.S.))
Resource5=IDR_MAINFRAME (English (U.S.))
Resource6=IDR_TB_TOOLBAR (English (U.S.))
Resource7=IDR_MAINFRAME (Chinese (Taiwan))
Resource8=IDD_ABOUTBOX (English (U.S.))
Resource9=IDD_DLG_OPENMSG (English (U.S.))
Resource10=IDR_TB_TOOLBAR2 (English (U.S.))

[CLS:CAviDlg]
Type=0
BaseClass=CDialog
HeaderFile=avidlg.h
ImplementationFile=avidlg.cpp
LastObject=CAviDlg

[CLS:CContactsDlg]
Type=0
BaseClass=CDialog
HeaderFile=contactsdlg.h
ImplementationFile=contactsdlg.cpp

[CLS:CLeftView]
Type=0
BaseClass=CTreeView
HeaderFile=leftview.h
ImplementationFile=leftview.cpp

[CLS:CMainFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=mainfrm.h
ImplementationFile=mainfrm.cpp

[CLS:CNewSMS]
Type=0
BaseClass=CDialog
HeaderFile=newsms.h
ImplementationFile=newsms.cpp

[CLS:COpenSMS]
Type=0
BaseClass=CDialog
HeaderFile=opensms.h
ImplementationFile=opensms.cpp

[CLS:CPhoneNumberEdit]
Type=0
BaseClass=CEdit
HeaderFile=phonenumberedit.h
ImplementationFile=phonenumberedit.cpp

[CLS:CPrevView]
Type=0
BaseClass=CListView
HeaderFile=prevview.h
ImplementationFile=prevview.cpp

[CLS:CProcessDlg]
Type=0
BaseClass=CDialog
HeaderFile=processdlg.h
ImplementationFile=processdlg.cpp

[CLS:CProgressDlg]
Type=0
BaseClass=CDialog
HeaderFile=ProgressDlg.h
ImplementationFile=ProgressDlg.cpp

[CLS:CReportCtrl]
Type=0
BaseClass=CListCtrl
HeaderFile=reportctrl.h
ImplementationFile=reportctrl.cpp

[CLS:CSkinHeaderCtrl]
Type=0
BaseClass=CHeaderCtrl
HeaderFile=skinheaderctrl.h
ImplementationFile=skinheaderctrl.cpp

[CLS:CSMSUtility]
Type=0
BaseClass=CWinApp
HeaderFile=smsutility.h
ImplementationFile=SMSUtility_DLL.cpp

[CLS:CTabSplitterWnd]
Type=0
BaseClass=CSplitterWnd
HeaderFile=tabsplitterwnd.h
ImplementationFile=tabsplitterwnd.cpp

[DLG:IDD_DLG_AVI]
Type=1
Class=CAviDlg

[DLG:IDD_DLG_CONTACTS]
Type=1
Class=CContactsDlg

[DLG:IDD_DLG_NEWSMS]
Type=1
Class=CNewSMS

[DLG:IDD_DLG_OPENMSG]
Type=1
Class=COpenSMS

[DLG:IDD_PROCESS_DLG]
Type=1
Class=CProcessDlg

[DLG:IDD_AVI_DLG]
Type=1
Class=CProgressDlg

[ACL:IDR_MAINFRAME (Chinese (Taiwan))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=?
ControlCount=4
Control1=IDC_ST_1,static,1342308480
Control2=IDC_ST_2,static,1342308352
Control3=IDOK,button,1342373899
Control4=IDC_ST_MAIN,static,1342308480

[DLG:IDD_DLG_OPENMSG (English (U.S.))]
Type=1
Class=?
ControlCount=10
Control1=IDC_EDIT_SENDER,edit,1350567936
Control2=IDC_EDIT_TIME,edit,1350567936
Control3=IDC_EDIT_CONTENT,edit,1352665092
Control4=IDC_BTN_PREV,button,1342177291
Control5=IDC_BTN_NEXT,button,1342177291
Control6=IDC_BTN_REPLY,button,1342177291
Control7=IDCANCEL,button,1342177291
Control8=IDC_ST_SENDER,static,1342308352
Control9=IDC_ST_TIME,static,1342308352
Control10=IDC_ST_MSGCONTENT,static,1342308352

[DLG:IDD_DLG_NEWSMS (English (U.S.))]
Type=1
Class=?
ControlCount=10
Control1=IDC_EDIT_CONTACTSLIST,edit,1350566016
Control2=IDC_EDIT_CONTENTS,edit,1352667140
Control3=IDC_BTN_CONTACTS,button,1342177291
Control4=IDC_BTN_SEND,button,1342177291
Control5=IDC_BTN_DRAFT,button,1342242827
Control6=IDCANCEL,button,1342177291
Control7=IDOK,button,1207959563
Control8=IDC_ST_RECEIVER,static,1342308352
Control9=IDC_ST_MSGCONTENT,static,1342308352
Control10=IDC_ST_COUNTS,static,1342177280

[DLG:IDD_DLG_CONTACTS (English (U.S.))]
Type=1
Class=?
ControlCount=6
Control1=IDOK,button,1342177291
Control2=IDCANCEL,button,1342177291
Control3=IDC_LIST_CONTACTS,SysListView32,1350600705
Control4=IDC_LIST_SELPHONE,SysListView32,1350600705
Control5=IDC_BTN_ADD,button,1342177291
Control6=IDC_BTN_DELETE,button,1342177291

[DLG:IDD_PROCESS_DLG (English (U.S.))]
Type=1
Class=?
ControlCount=3
Control1=IDC_PROGRESS_BAR,msctls_progress32,1350565889
Control2=IDC_STOP,button,1342242827
Control3=IDC_STATIC_MSG,static,1342308352

[DLG:IDD_AVI_DLG (English (U.S.))]
Type=1
Class=?
ControlCount=2
Control1=IDCANCEL,button,1342242827
Control2=IDC_ANIMATE,SysAnimate32,1342242820

[MNU:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_BN_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_PRINT
Command6=ID_FILE_PRINT_PREVIEW
Command7=ID_FILE_PRINT_SETUP
Command8=ID_FILE_MRU_FILE1
Command9=ID_APP_EXIT
Command10=ID_EDIT_UNDO
Command11=ID_EDIT_CUT
Command12=ID_EDIT_COPY
Command13=ID_EDIT_PASTE
Command14=ID_VIEW_TOOLBAR
Command15=ID_VIEW_STATUS_BAR
Command16=ID_APP_ABOUT
CommandCount=16

[TB:IDR_TB_TOOLBAR (English (U.S.))]
Type=1
Class=?
Command1=ID_BN_NEW
Command2=ID_BN_REPLY
Command3=ID_BN_SAVE
Command4=ID_BN_PRINT
Command5=ID_BN_DELETE
Command6=ID_BN_RESEND
Command7=ID_BN_REFRESH
CommandCount=7

[TB:IDR_TB_TOOLBAR2 (English (U.S.))]
Type=1
Class=?
Command1=ID_BN_BACK
Command2=ID_BN_ABOUT
Command3=ID_BN_HELP
CommandCount=3

