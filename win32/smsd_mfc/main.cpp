
#include "common.h"

#include <afxwin.h>
#include <afxext.h>
#include <afxcview.h>
#include <windows.h>
#include <winsock.h>
#include <mysql.h>

#include "resource.h"
#include "loguser.h"
#include "sendsms.h"

#pragma comment(lib, "libmysql.lib")

char  *server   = new char(200);
char  *user     = new char(200);
char  *password = new char(200);

//--document in each child window

class CMultiDoc : public CDocument
{
protected:
	DECLARE_DYNCREATE(CMultiDoc)
public:
	CString FFolder;
	CString FDB;
	MYSQL 	DB;
	virtual BOOL OnNewDocument() {
		return CDocument::OnNewDocument();
	}
};

IMPLEMENT_DYNCREATE(CMultiDoc, CDocument)

//--- left side of child windows

class CSplitTreeView : public CTreeView
{ 
protected:
	CSplitTreeView();
	~CSplitTreeView();
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);	
	DECLARE_DYNCREATE(CSplitTreeView)
public:
	virtual void OnInitialUpdate();
	CMultiDoc* GetDocument() const;
	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNCREATE(CSplitTreeView, CTreeView)

BEGIN_MESSAGE_MAP(CSplitTreeView, CTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
END_MESSAGE_MAP()

inline CMultiDoc* CSplitTreeView::GetDocument() const
{ 
	return reinterpret_cast<CMultiDoc*>(m_pDocument);
}

CSplitTreeView::CSplitTreeView()
{
	this->m_dwDefaultStyle |= (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS);
}


CSplitTreeView::~CSplitTreeView()
{
//	if (GetDocument()->DB!=NULL) 
	mysql_close(&GetDocument()->DB);
}

void CSplitTreeView::OnInitialUpdate()
{
	HTREEITEM 	hChapter;
	char		*buf="SELECT Version FROM `gammu` WHERE 1";
	MYSQL_RES 	*Res0, *Res;
	MYSQL_ROW 	Row0, Row;

    	CTreeView::OnInitialUpdate();
    	CTreeCtrl& tree = GetTreeCtrl();

	mysql_init(&GetDocument()->DB);

	if (!mysql_real_connect(&GetDocument()->DB,server,user,password,NULL,0,NULL,0)) {
//	if (GetDocument()->DB == NULL) {
		MessageBox("","Connection error");
		return;
	}

	if (!(Res0 = mysql_list_dbs(&GetDocument()->DB,NULL))) {
		MessageBox(mysql_error(&GetDocument()->DB),"Getting DB list error");
		return;
	}
	while ((Row0 = mysql_fetch_row(Res0))) {
		if (mysql_select_db(&GetDocument()->DB, Row0[0])!=0) {
			mysql_free_result(Res);
			continue;
		}
		if (mysql_real_query(&GetDocument()->DB,buf,strlen(buf))) {
			continue;
		}
		if (!(Res = mysql_store_result(&GetDocument()->DB))) {
			continue;
		}

		if (!(Row = mysql_fetch_row(Res))) {
			mysql_free_result(Res);
			continue;
		}
		if (atoi(Row[0]) != 3) {
			mysql_free_result(Res);
			continue;
		}
//		MessageBox(Row0[0]);
		hChapter = tree.InsertItem(Row0[0]);
	    	tree.InsertItem( "Phones", hChapter);
	    	tree.InsertItem( "Inbox", hChapter);
	    	tree.InsertItem( "Outbox", hChapter );
	    	tree.InsertItem( "Sent items", hChapter );
		mysql_free_result(Res);		
	}
	mysql_free_result(Res);
}

void CSplitTreeView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* 	pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	CTreeCtrl 	&ctrl = GetTreeCtrl();
	HTREEITEM 	hNew = ctrl.GetSelectedItem();

	GetDocument()->FDB = "";
	if (hNew != NULL) {
		CString pStr = ctrl.GetItemText(hNew);
		GetDocument()->FFolder = pStr;
		HTREEITEM hParent = ctrl.GetParentItem(hNew);
		if (hParent != NULL) {
			pStr = ctrl.GetItemText(hParent);
			if (pStr != GetDocument()->FDB) {
//				MessageBox(pStr,"Selecting DB");
				if (mysql_select_db(&GetDocument()->DB, pStr)!=0) {
					MessageBox(mysql_error(&GetDocument()->DB),"Selecting DB error");
				}
			}
			GetDocument()->FDB = pStr;
		}
	} else {
		GetDocument()->FFolder = "";
	}
	GetDocument()->UpdateAllViews(this);

	*pResult = 0;
}

//--- right up side of child windows

class CSplitListView : public CListView
{ 
protected:
	CSplitListView();
	DECLARE_DYNCREATE(CSplitListView)
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
public:
	CMultiDoc* GetDocument() const;
	virtual void OnInitialUpdate();
};

IMPLEMENT_DYNCREATE(CSplitListView, CListView)

inline CMultiDoc* CSplitListView::GetDocument() const
{ 
	return reinterpret_cast<CMultiDoc*>(m_pDocument);
}

char *DecodeValidity(char *Validity)
{
	static char buffer[100];

	if (!strcmp(Validity,"-1")) {
		sprintf(buffer,"default");
	} else if (!strcmp(Validity,"0")) {
		sprintf(buffer,"5 minutes");
	} else if (!strcmp(Validity,"255")) {
		sprintf(buffer,"max. time");
	} else {
		sprintf(buffer,Validity);
	}
	return buffer;
}

void CSplitListView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	char		*inbox="SELECT UpdatedInDB,ReceivingDateTime,Text,SenderNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,RecipientID FROM `inbox` WHERE 1";
	char		*outbox="SELECT UpdatedInDB,InsertIntoDB,Text,DestinationNumber,Coding,UDH,Class,TextDecoded,ID,MultiPart,RelativeValidity,SendingDateTime,SenderID,SendingTimeOut FROM `outbox` WHERE 1";
	char		*sent="SELECT UpdatedInDB,InsertIntoDB,SendingDateTime,DeliveryDateTime,Text,DestinationNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,SenderID,SequencePosition,Status,StatusError,TPMR,RelativeValidity FROM `sentitems` order by SendingDateTime";
	char		*phones="SELECT ID,InsertIntoDB,TimeOut,Send,Receive,IMEI FROM `phones` WHERE TimeOut>NOW()";
	LV_COLUMN 	lvc;
	CString 	pStr = GetDocument()->FFolder;
	CListCtrl& 	pmyListCtrl = GetListCtrl();
	MYSQL_RES 	*Res;
	MYSQL_ROW 	Row;
	int		index;

	pmyListCtrl.SetExtendedStyle(0);

	if (pmyListCtrl.GetItemCount() != 0) pmyListCtrl.DeleteAllItems();

	int Col = pmyListCtrl.GetHeaderCtrl()->GetItemCount();
	for (int i=0;i<Col;i++) pmyListCtrl.DeleteColumn(0);

//	LPTSTR lpsz = new TCHAR[pStr.GetLength()+1];
//	_tcscpy(lpsz, pStr);

	lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
		
	if (pStr.Compare("Phones")==0) {
		pmyListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

		lvc.iSubItem = 0;
//		lvc.pszText = lpsz;
		lvc.pszText = _T("ID");
		lvc.cx = 70;
		pmyListCtrl.InsertColumn(0,&lvc);

		lvc.iSubItem = 1;
		lvc.pszText = _T("Sending SMS");
		lvc.cx = 80;
		pmyListCtrl.InsertColumn(1,&lvc);

		lvc.iSubItem = 2;
		lvc.pszText = _T("Receiving SMS");
		lvc.cx = 100;
		pmyListCtrl.InsertColumn(2,&lvc);

		lvc.iSubItem = 3;
		lvc.pszText = _T("IMEI");
		lvc.cx = 150;
		pmyListCtrl.InsertColumn(3,&lvc);

		lvc.iSubItem = 3;
		lvc.pszText = _T("Logged");
		lvc.cx = 150;
		pmyListCtrl.InsertColumn(3,&lvc);

		if (mysql_real_query(&GetDocument()->DB,phones,strlen(phones))) {
			MessageBox(mysql_error(&GetDocument()->DB),"Query error");
			return;
		}
		if (!(Res = mysql_store_result(&GetDocument()->DB))) {
			MessageBox(mysql_error(&GetDocument()->DB),"Store error");
			return;
		}
		while ((Row = mysql_fetch_row(Res))) {
			index = pmyListCtrl.InsertItem(0,Row[0]); //ID
			pmyListCtrl.SetItemText(index,1,Row[3]);  //Sending
			pmyListCtrl.SetItemText(index,2,Row[4]);  //Receiving
			pmyListCtrl.SetItemText(index,3,Row[1]);  //Logged
			pmyListCtrl.SetItemText(index,4,Row[5]);  //IMEI
		}
		mysql_free_result(Res);
	}
	if (pStr.Compare("Inbox")==0) {
		pmyListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

		lvc.iSubItem = 0;
//		lvc.pszText = lpsz;
		lvc.pszText = _T("ID");
		lvc.cx = 70;
		pmyListCtrl.InsertColumn(0,&lvc);

		lvc.iSubItem = 1;
		lvc.pszText = _T("From");
		lvc.cx = 125;
		pmyListCtrl.InsertColumn(1,&lvc);

		lvc.iSubItem = 2;
		lvc.pszText = _T("SMSC");
		lvc.cx = 125;
		pmyListCtrl.InsertColumn(2,&lvc);

		lvc.iSubItem = 3;
		lvc.pszText = _T("Class");
		lvc.cx = 50;
		pmyListCtrl.InsertColumn(3,&lvc);

		lvc.iSubItem = 4;
		lvc.pszText = _T("Receive Time");
		lvc.cx = 125;
		pmyListCtrl.InsertColumn(4,&lvc);

		lvc.iSubItem = 5;
		lvc.pszText = _T("Phone");
		lvc.cx = 125;
		pmyListCtrl.InsertColumn(5,&lvc);

		if (mysql_real_query(&GetDocument()->DB,inbox,strlen(inbox))) {
			MessageBox(mysql_error(&GetDocument()->DB),"Query error");
			return;
		}
		if (!(Res = mysql_store_result(&GetDocument()->DB))) {
			MessageBox(mysql_error(&GetDocument()->DB),"Store error");
			return;
		}
		while ((Row = mysql_fetch_row(Res))) {
			index = pmyListCtrl.InsertItem(0,Row[9]); //ID
			pmyListCtrl.SetItemText(index,1,Row[3]);  //From
			pmyListCtrl.SetItemText(index,2,Row[6]);  //SMSC
			pmyListCtrl.SetItemText(index,3,Row[7]);  //class
			pmyListCtrl.SetItemText(index,4,Row[1]);  //receive time
			pmyListCtrl.SetItemText(index,4,Row[10]); //phone
		}
		mysql_free_result(Res);
	}
	if (pStr.Compare("Outbox")==0) {
		pmyListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

		lvc.iSubItem = 0;
		lvc.pszText = _T("ID");
		lvc.cx = 70;
		pmyListCtrl.InsertColumn(0,&lvc);

		lvc.iSubItem = 1;
		lvc.pszText = _T("To");
		lvc.cx = 125;
		pmyListCtrl.InsertColumn(1,&lvc);

		lvc.iSubItem = 2;
		lvc.pszText = _T("Time 2be sent");
		lvc.cx = 125;
		pmyListCtrl.InsertColumn(2,&lvc);

		lvc.iSubItem = 3;
		lvc.pszText = _T("Parts");
		lvc.cx = 50;
		pmyListCtrl.InsertColumn(3,&lvc);

		lvc.iSubItem = 4;
		lvc.pszText = _T("Validity");
		lvc.cx = 50;
		pmyListCtrl.InsertColumn(4,&lvc);

		lvc.iSubItem = 5;
		lvc.pszText = _T("Sending");
		lvc.cx = 110;
		pmyListCtrl.InsertColumn(5,&lvc);

		lvc.iSubItem = 6;
		lvc.pszText = _T("Phone");
		lvc.cx = 115;
		pmyListCtrl.InsertColumn(6,&lvc);

		if (mysql_real_query(&GetDocument()->DB,outbox,strlen(outbox))) {
			MessageBox(mysql_error(&GetDocument()->DB),"Query error");
			return;
		}
		if (!(Res = mysql_store_result(&GetDocument()->DB))) {
			MessageBox(mysql_error(&GetDocument()->DB),"Store error");
			return;
		}
		while ((Row = mysql_fetch_row(Res))) {
			index = pmyListCtrl.InsertItem(0,Row[8]);//ID
			pmyListCtrl.SetItemText(index,1,Row[3]); //To
			pmyListCtrl.SetItemText(index,2,Row[11]);//Time2sent
			if (!strcmp(Row[9],"false")) {
				pmyListCtrl.SetItemText(index,3,"1");	 //parts
			} else {
				pmyListCtrl.SetItemText(index,3,"a");	 //parts
			}
			pmyListCtrl.SetItemText(index,4,DecodeValidity(Row[10]));//validity
			pmyListCtrl.SetItemText(index,5,"a");	 //sending
			pmyListCtrl.SetItemText(index,6,Row[12]);//phone
		}
		mysql_free_result(Res);
	}	
	if (pStr.Compare("Sent items")==0) {
		pmyListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

		lvc.iSubItem = 0;
		lvc.pszText = _T("ID");
		lvc.cx = 70;
		pmyListCtrl.InsertColumn(0,&lvc);

		lvc.iSubItem = 1;
		lvc.pszText = _T("To");
		lvc.cx = 125;
		pmyListCtrl.InsertColumn(1,&lvc);

		lvc.iSubItem = 2;
		lvc.pszText = _T("SMSC");
		lvc.cx = 125;
		pmyListCtrl.InsertColumn(2,&lvc);

		lvc.iSubItem = 3;
		lvc.pszText = _T("Send time");
		lvc.cx = 125;
		pmyListCtrl.InsertColumn(3,&lvc);

		lvc.iSubItem = 4;
		lvc.pszText = _T("Phone");
		lvc.cx = 50;
		pmyListCtrl.InsertColumn(4,&lvc);

		if (mysql_real_query(&GetDocument()->DB,sent,strlen(sent))) {
			MessageBox(mysql_error(&GetDocument()->DB),"Query error");
			return;
		}
		if (!(Res = mysql_store_result(&GetDocument()->DB))) {
			MessageBox(mysql_error(&GetDocument()->DB),"Store error");
			return;
		}
		while ((Row = mysql_fetch_row(Res))) {
			index = pmyListCtrl.InsertItem(0,Row[11]);//ID
			pmyListCtrl.SetItemText(index,1,Row[5]);  //To
			pmyListCtrl.SetItemText(index,2,Row[8]);  //SMSC
			pmyListCtrl.SetItemText(index,3,Row[2]);  //when sent
			pmyListCtrl.SetItemText(index,4,Row[12]); //phone
		}
		mysql_free_result(Res);
	}
}

CSplitListView::CSplitListView()
{
	m_dwDefaultStyle |= (LVS_REPORT);
}

void CSplitListView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();
}

//--- child windows inside main frame

class CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	void CChildFrame::ActivateFrame(int nCmdShow);
protected:
	CSplitterWnd 	m_wndSplitter;
	CSplitterWnd 	m_wndSplitter2;
};

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	if (!m_wndSplitter.CreateStatic(this, 1, 2)) return FALSE;
	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CSplitTreeView), CSize(130, 50), pContext)) return FALSE;

	if (!m_wndSplitter2.CreateStatic(&m_wndSplitter, 2, 1, WS_CHILD | WS_VISIBLE | WS_BORDER, m_wndSplitter.IdFromRowCol(0, 1))) return FALSE;

	int cyText = max(lpcs->cy - 70, 20);

	if (!m_wndSplitter2.CreateView(0, 0, RUNTIME_CLASS(CSplitListView), CSize(0, cyText), pContext)) return FALSE;
	if (!m_wndSplitter2.CreateView(1, 0, RUNTIME_CLASS(CSplitListView), CSize(0, 0), pContext)) 	 return FALSE;

	return TRUE;
}

void CChildFrame::ActivateFrame(int nCmdShow) 
{
	nCmdShow = SW_MAXIMIZE;   
	CMDIChildWnd::ActivateFrame(nCmdShow);
}

//---- main frame

static UINT indicators[] =
{
	ID_SEPARATOR,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

class CMainFrame:public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
protected:
	CStatusBar StatusBar1;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct) {
		if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;

		StatusBar1.Create(this);
		StatusBar1.SetIndicators(indicators,sizeof(indicators)/sizeof(UINT));

		return 0;
	}
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus) {
		CMDIFrameWnd::OnShowWindow(bShow, nStatus);
	
		ShowWindow(SW_MAXIMIZE);
	}
public:
	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

//-- application

class CMyApp:public CWinApp
{
public:
	BOOL InitInstance() {
		CMultiDocTemplate* 	pDocTemplate;
		CMDIFrameWnd* 		pMainFrame;

		InitCommonControls();

		pDocTemplate = new CMultiDocTemplate(IDR_TXT,
			RUNTIME_CLASS(CMultiDoc),
			RUNTIME_CLASS(CChildFrame),
			RUNTIME_CLASS(CSplitTreeView));
		if (!pDocTemplate) return FALSE;
		AddDocTemplate(pDocTemplate);

		pMainFrame = new CMainFrame;
		if (pMainFrame==NULL) return FALSE;		
		if (!pMainFrame->LoadFrame(IDR_MAIN)) return FALSE;
		pMainFrame->ShowWindow(m_nCmdShow);
		pMainFrame->UpdateWindow();
		m_pMainWnd = pMainFrame;

		return TRUE;
	}
	void OnClickExit() {
		exit(0);
	}
	void OnClickNew() {
		CLogUserDlg dlg;
		if (dlg.DoModal()!=IDOK) return;

		_tcscpy(server, dlg.IP);
		_tcscpy(user, dlg.User);
		_tcscpy(password, dlg.Pass);
		
		CWinApp::OnFileNew();
	}
	void OnClickSendSMS() {
		CSendSMSDlg dlg;
		dlg.DoModal();
	}
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CMyApp, CWinApp)
	ON_COMMAND(ID_FILE_EXIT,OnClickExit)
	ON_COMMAND(ID_FILE_NEW,OnClickNew)
	ON_COMMAND(ID_TOOLS_SENDSMS,OnClickSendSMS)
END_MESSAGE_MAP()

CMyApp theApp;
