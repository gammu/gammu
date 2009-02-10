
#include "common.h"

#include <afx.h>
#include <afxwin.h>
#include <afxext.h>
#include <afxcview.h>
#include <afxrich.h>
#include <afxtempl.h>

#include <windows.h>
#include <winsock.h>
#include <mysql.h>
#include <locale.h>

#include "../../common/misc/coding/coding.h"

#include "resource.h"
#include "loguser.h"
#include "sendsms.h"
#include "editpbk.h"
#include "config.h"
#include "about.h"

#pragma comment(lib, "libmysql.lib")

char  	*server   = new char(200);
char  	*user     = new char(200);
char  	*password = new char(200);
MYSQL 	MainDB;
HWND  	MainHWND;

void DispSMSInfo(CString *txt, char *Class, char *UDH, char *Text, char *TextDecoded, char *Coding)
{
	unsigned char buf[1000],buf2[1000];

	if (UDH[0] != 0) (*txt) = (*txt) + "UDH AVAILABLE\r\n";
	if (!strcmp(Class,"0") || !strcmp(Class,"1") || !strcmp(Class,"2") || !strcmp(Class,"3")) {
		(*txt) = (*txt) + "Class: "+Class+"\r\n";
	}
	if (!strcmp(Coding,"8bit")) {
		(*txt) = (*txt) + "BINARY\r\n";
	} else {
		if (!strcmp(Coding,"Unicode")) (*txt) = (*txt) + "Unicode ";
		(*txt) = (*txt) + "text: ";
		if (Text[0] == 0) {
			(*txt) = (*txt) + TextDecoded;
		} else {
			DecodeHexUnicode(buf, (unsigned char *)Text, strlen(Text));
  			(*txt) = (*txt) + DecodeUnicodeString(buf);
		}
	}
}

char *DecodeDateTime(char *DT)
{
	static char buffer[100];

	if (strlen(DT)<14) {
		buffer[0] = 0;
	} else {
		sprintf(buffer,"%c%c%c%c-%c%c-%c%c %c%c:%c%c:%c%c",
			DT[0],DT[1],DT[2],DT[3],DT[4],
			DT[5],DT[6],
			DT[7],DT[8],
			DT[9],DT[10],
			DT[11],DT[12],
			DT[13],DT[14]);
	}
	
	return buffer;
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

//--document in each child window

class CMultiDoc : public CDocument
{
protected:
	DECLARE_DYNCREATE(CMultiDoc)
public:
	CString FFolder;
	CString FDB;
	CString FID;
	MYSQL 	*DB;
	virtual BOOL OnNewDocument() {
		DB = NULL;
 		unsigned int port = 0;

		if (!CDocument::OnNewDocument()) return FALSE;

 		char * pport = strstr( server, ":" );
 		if (pport) {
 			*pport ++ = _T('\0');
 			port = atoi( pport );
 		} 
 		DB = mysql_real_connect(&MainDB,server,user,password,NULL,port,NULL,0);
		if (DB == NULL) {
			MessageBox(MainHWND,mysql_error(&MainDB),"Connection error",0);
			return FALSE;
		}
		return TRUE;
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
	if (GetDocument()->DB!=NULL) mysql_close(GetDocument()->DB);
}

void CSplitTreeView::OnInitialUpdate()
{
	HTREEITEM 	hChapter;
	char		*buf="SELECT Version FROM `gammu` WHERE 1";
	MYSQL_RES 	*Res0, *Res;
	MYSQL_ROW 	Row0, Row;

    	CTreeView::OnInitialUpdate();
    	CTreeCtrl& tree = GetTreeCtrl();

	if (!(Res0 = mysql_list_dbs(GetDocument()->DB,NULL))) {
		MessageBox(mysql_error(GetDocument()->DB),"Getting DB list error");
		return;
	}
	while ((Row0 = mysql_fetch_row(Res0))) {
		if (mysql_select_db(GetDocument()->DB, Row0[0])!=0) {
			mysql_free_result(Res);
			continue;
		}
		if (mysql_real_query(GetDocument()->DB,buf,strlen(buf))) {
			continue;
		}
		if (!(Res = mysql_store_result(GetDocument()->DB))) {
			continue;
		}

		if (!(Row = mysql_fetch_row(Res))) {
			mysql_free_result(Res);
			continue;
		}
		if (atoi(Row[0]) != 4) {
			mysql_free_result(Res);
			continue;
		}
		hChapter = tree.InsertItem(Row0[0]);
	    	tree.InsertItem( "Phones", hChapter);
	    	tree.InsertItem( "Inbox", hChapter);
	    	tree.InsertItem( "Outbox", hChapter );
	    	tree.InsertItem( "Sent items", hChapter );
	}
	mysql_free_result(Res);
}

void CSplitTreeView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* 	pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	CTreeCtrl 	&ctrl = GetTreeCtrl();
	HTREEITEM 	hNew = ctrl.GetSelectedItem();

	GetDocument()->FDB = "";
	GetDocument()->FID = "";
	if (hNew != NULL) {
		CString pStr = ctrl.GetItemText(hNew);
		GetDocument()->FFolder = pStr;
		HTREEITEM hParent = ctrl.GetParentItem(hNew);
		if (hParent != NULL) {
			pStr = ctrl.GetItemText(hParent);
			if (pStr != GetDocument()->FDB) {
				if (mysql_select_db(GetDocument()->DB, pStr)!=0) {
					MessageBox(mysql_error(GetDocument()->DB),"Selecting DB error");
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

//--- right down side of child windows

class CSplitEditView : public CRichEditView
{
protected:
	CSplitEditView();
	DECLARE_DYNCREATE(CSplitEditView)
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
public:
	CMultiDoc* GetDocument() const;
	virtual void OnInitialUpdate();
};

IMPLEMENT_DYNCREATE(CSplitEditView, CEditView)

CSplitEditView::CSplitEditView()
{
}

void CSplitEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CString 		txt;
	CRichEditCtrl& 		theEdit = GetRichEditCtrl();
	char			*inbox="SELECT UpdatedInDB,ReceivingDateTime,Text,SenderNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,RecipientID FROM `inbox` WHERE ID=";
	char			*outbox="SELECT UpdatedInDB,InsertIntoDB,Text,DestinationNumber,Coding,UDH,Class,TextDecoded,ID,MultiPart,RelativeValidity,SendingDateTime,SenderID,SendingTimeOut FROM `outbox` WHERE ID=";
	char			*outbox_multi="SELECT Text,Coding,UDH,Class,TextDecoded,ID,SequencePosition FROM `outbox_multipart` WHERE ID=";
	char			*sent="SELECT UpdatedInDB,InsertIntoDB,SendingDateTime,DeliveryDateTime,Text,DestinationNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,SenderID,SequencePosition,Status,StatusError,TPMR,RelativeValidity FROM `sentitems` WHERE ID=";
	char			*phones="SELECT ID,InsertIntoDB,TimeOut,Send,Receive,IMEI,Client FROM `phones` WHERE TimeOut>NOW()";
	char			something[1000];
	MYSQL_RES 		*Res;
	MYSQL_ROW 		Row;
	int 			i;

	txt = "";
	theEdit.SetWindowText(txt);
	something[0] = 0;
	
	if (GetDocument()->FID == "") return;

	if (GetDocument()->FFolder=="Inbox") {
		strcpy(something,inbox);
		sprintf(something+strlen(something),"%s",GetDocument()->FID);
		if (mysql_real_query(GetDocument()->DB,something,strlen(something))) {
			MessageBox(mysql_error(GetDocument()->DB),"Query error");
			return;
		}

		if (!(Res = mysql_store_result(GetDocument()->DB))) {
			MessageBox(mysql_error(GetDocument()->DB),"Store error");
			return;
		}
		Row = mysql_fetch_row(Res);

		txt = "Last changed in DB: ";
		txt = txt + DecodeDateTime(Row[0]);
		txt = txt + "\r\nInsert into DB: " + DecodeDateTime(Row[1]);
		txt = txt + "\r\n";

		DispSMSInfo(&txt, Row[7], Row[5], Row[2], Row[8], Row[4]);

		mysql_free_result(Res);
	}
	if (GetDocument()->FFolder=="Outbox") {
		strcpy(something,outbox);
		sprintf(something+strlen(something),"%s",GetDocument()->FID);
		if (mysql_real_query(GetDocument()->DB,something,strlen(something))) {
			MessageBox(mysql_error(GetDocument()->DB),"Query error");
			return;
		}

		if (!(Res = mysql_store_result(GetDocument()->DB))) {
			MessageBox(mysql_error(GetDocument()->DB),"Store error");
			return;
		}
		Row = mysql_fetch_row(Res);

		txt = "Last changed in DB: ";
		txt = txt + DecodeDateTime(Row[0]);
		txt = txt + "\r\nInsert into DB: " + DecodeDateTime(Row[1]);
		txt = txt + "\r\n";

		DispSMSInfo(&txt, Row[6], Row[5], Row[2], Row[7], Row[4]);

		mysql_free_result(Res);

		if (!strcmp(Row[9],"true")) {
			strcpy(something,outbox_multi);
			sprintf(something+strlen(something),"%s",GetDocument()->FID);
			if (mysql_real_query(GetDocument()->DB,something,strlen(something))) {
				MessageBox(mysql_error(GetDocument()->DB),"Query error");
				return;
			}

			if (!(Res = mysql_store_result(GetDocument()->DB))) {
				MessageBox(mysql_error(GetDocument()->DB),"Store error");
				return;
			}
			Row = mysql_fetch_row(Res);

			DispSMSInfo(&txt, Row[3], Row[2], Row[0], Row[4], Row[1]);
	
			mysql_free_result(Res);
		}
	}
	if (GetDocument()->FFolder=="Sent items") {
		for (i=1;i<10;i++) {
			strcpy(something,sent);
			sprintf(something+strlen(something),"%s",GetDocument()->FID);
			sprintf(something+strlen(something)," and SequencePosition=%i limit 1",i);
			if (mysql_real_query(GetDocument()->DB,something,strlen(something))) {
				MessageBox(mysql_error(GetDocument()->DB),"Query error");
				return;
			}
	
			if (!(Res = mysql_store_result(GetDocument()->DB))) {
				MessageBox(mysql_error(GetDocument()->DB),"Store error");
				return;
			}
			Row = mysql_fetch_row(Res);
			if (Row == NULL) {
				mysql_free_result(Res);
				break;
			}

			txt = "Validity: ";
			txt = txt + DecodeValidity(Row[17]);
			txt = txt + "\r\n";

			DispSMSInfo(&txt, Row[9], Row[7], Row[4], Row[10], Row[6]);

			mysql_free_result(Res);
		}
		strcpy(something,sent);
		sprintf(something+strlen(something),"%s",GetDocument()->FID);
		if (mysql_real_query(GetDocument()->DB,something,strlen(something))) {
			MessageBox(mysql_error(GetDocument()->DB),"Query error");
			return;
		}
	
		if (!(Res = mysql_store_result(GetDocument()->DB))) {
			MessageBox(mysql_error(GetDocument()->DB),"Store error");
			return;
		}
		txt = txt + "\r\n";
		while ((Row = mysql_fetch_row(Res))) {
			txt = txt + "\r\n";
			txt = txt + "Part ";
			txt = txt + Row[13];
			txt = txt + "\r\n";
			if (strcmp(Row[15],"-1")) {
				txt = txt + "Error code ";
				txt = txt + Row[15];
				txt = txt + "\r\n";
			}
			txt = txt + "Status ";
			txt = txt + Row[14];
			txt = txt + "\r\n";
			txt = txt + "Sending time ";
			txt = txt + DecodeDateTime(Row[2]);
			txt = txt + "\r\n";
			if (strcmp(Row[3],"00000000000000")) {
				txt = txt + "Delivery time ";
				txt = txt + DecodeDateTime(Row[3]);
				txt = txt + "\r\n";
			}
		}
		mysql_free_result(Res);
	}

	theEdit.SetWindowText (txt);
}

void CSplitEditView::OnInitialUpdate()
{
	CRichEditCtrl& 	theEdit = GetRichEditCtrl();
	
	theEdit.SetReadOnly(TRUE);
}

inline CMultiDoc* CSplitEditView::GetDocument() const
{ 
	return reinterpret_cast<CMultiDoc*>(m_pDocument);
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
	void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNCREATE(CSplitListView, CListView)

BEGIN_MESSAGE_MAP(CSplitListView, CListView)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
END_MESSAGE_MAP()

void CSplitListView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CListCtrl& 	pmyListCtrl = GetListCtrl();
	CString		Item;
	CString 	pStr = GetDocument()->FFolder;
	
	GetDocument()->FID = "";

	POSITION Pos = pmyListCtrl.GetFirstSelectedItemPosition();
	if (Pos) {
		int nItem = pmyListCtrl.GetNextSelectedItem(Pos);
		if (pStr == "Inbox" || pStr == "Outbox" || pStr == "Sent items") {
			GetDocument()->FID = pmyListCtrl.GetItemText(nItem,0);
		}
	}

	GetDocument()->UpdateAllViews(this);
}

inline CMultiDoc* CSplitListView::GetDocument() const
{ 
	return reinterpret_cast<CMultiDoc*>(m_pDocument);
}

void CSplitListView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	LV_COLUMN 	lvc;
	CString 	pStr = GetDocument()->FFolder;
	CListCtrl& 	pmyListCtrl = GetListCtrl();
	MYSQL_RES 	*Res;
	MYSQL_ROW 	Row;
	int		index, counter;
	char		*inbox="SELECT UpdatedInDB,ReceivingDateTime,Text,SenderNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,RecipientID FROM `inbox` WHERE 1";
	char		*outbox="SELECT UpdatedInDB,InsertIntoDB,Text,DestinationNumber,Coding,UDH,Class,TextDecoded,ID,MultiPart,RelativeValidity,SendingDateTime,SenderID,SendingTimeOut,DeliveryReport FROM `outbox` WHERE 1";
	char		*outbox_multi="SELECT Text,Coding,UDH,Class,TextDecoded,ID,SequencePosition FROM `outbox_multipart` WHERE ID=";
	char		*sent="SELECT UpdatedInDB,InsertIntoDB,SendingDateTime,DeliveryDateTime,Text,DestinationNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,SenderID,SequencePosition,Status,StatusError,TPMR,RelativeValidity FROM `sentitems` order by SendingDateTime";
	char		*phones="SELECT ID,InsertIntoDB,TimeOut,Send,Receive,IMEI,Client FROM `phones` WHERE TimeOut>NOW()";
	char		something[1000];

	pmyListCtrl.SetExtendedStyle(0);

	if (pmyListCtrl.GetItemCount() != 0) pmyListCtrl.DeleteAllItems();

	int Col = pmyListCtrl.GetHeaderCtrl()->GetItemCount();
	for (int i=0;i<Col;i++) pmyListCtrl.DeleteColumn(0);

//	LPTSTR lpsz = new TCHAR[pStr.GetLength()+1];
//	_tcscpy(lpsz, pStr);

	lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
		
	if (pStr=="Phones") {
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
		lvc.cx = 80;
		pmyListCtrl.InsertColumn(2,&lvc);

		lvc.iSubItem = 3;
		lvc.pszText = _T("IMEI");
		lvc.cx = 105;
		pmyListCtrl.InsertColumn(3,&lvc);

		lvc.iSubItem = 4;
		lvc.pszText = _T("Logged");
		lvc.cx = 150;
		pmyListCtrl.InsertColumn(4,&lvc);

		lvc.iSubItem = 5;
		lvc.pszText = _T("Client");
		lvc.cx = 155;
		pmyListCtrl.InsertColumn(5,&lvc);

		if (mysql_real_query(GetDocument()->DB,phones,strlen(phones))) {
			MessageBox(mysql_error(GetDocument()->DB),"Query error");
			return;
		}
		if (!(Res = mysql_store_result(GetDocument()->DB))) {
			MessageBox(mysql_error(GetDocument()->DB),"Store error");
			return;
		}
		while ((Row = mysql_fetch_row(Res))) {
			index = pmyListCtrl.InsertItem(0,Row[0]); //ID
			pmyListCtrl.SetItemText(index,1,Row[3]);  //Sending
			pmyListCtrl.SetItemText(index,2,Row[4]);  //Receiving
			pmyListCtrl.SetItemText(index,3,Row[5]);  //IMEI
			pmyListCtrl.SetItemText(index,4,DecodeDateTime(Row[1]));  //Logged
			pmyListCtrl.SetItemText(index,5,Row[6]);  //Client
		}
		mysql_free_result(Res);
	}
	if (pStr=="Inbox") {
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

		if (mysql_real_query(GetDocument()->DB,inbox,strlen(inbox))) {
			MessageBox(mysql_error(GetDocument()->DB),"Query error");
			return;
		}
		if (!(Res = mysql_store_result(GetDocument()->DB))) {
			MessageBox(mysql_error(GetDocument()->DB),"Store error");
			return;
		}
		while ((Row = mysql_fetch_row(Res))) {
			index = pmyListCtrl.InsertItem(0,Row[9]); //ID
			pmyListCtrl.SetItemText(index,1,Row[3]);  //From
			pmyListCtrl.SetItemText(index,2,Row[6]);  //SMSC
			pmyListCtrl.SetItemText(index,3,Row[7]);  //class
			pmyListCtrl.SetItemText(index,4,DecodeDateTime(Row[1]));  //receive time
			pmyListCtrl.SetItemText(index,5,Row[10]); //phone
		}
		mysql_free_result(Res);
	}
	if (pStr=="Outbox") {
		pmyListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

		lvc.iSubItem = 0;
		lvc.pszText = _T("ID");
		lvc.cx = 70;
		pmyListCtrl.InsertColumn(0,&lvc);

		lvc.iSubItem = 1;
		lvc.pszText = _T("To");
		lvc.cx = 110;
		pmyListCtrl.InsertColumn(1,&lvc);

		lvc.iSubItem = 2;
		lvc.pszText = _T("Time 2be sent");
		lvc.cx = 125;
		pmyListCtrl.InsertColumn(2,&lvc);

		lvc.iSubItem = 3;
		lvc.pszText = _T("Parts");
		lvc.cx = 40;
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
		lvc.cx = 100;
		pmyListCtrl.InsertColumn(6,&lvc);

		lvc.iSubItem = 6;
		lvc.pszText = _T("Report");
		lvc.cx = 52;
		pmyListCtrl.InsertColumn(7,&lvc);

		if (mysql_real_query(GetDocument()->DB,outbox,strlen(outbox))) {
			MessageBox(mysql_error(GetDocument()->DB),"Query error");
			return;
		}
		if (!(Res = mysql_store_result(GetDocument()->DB))) {
			MessageBox(mysql_error(GetDocument()->DB),"Store error");
			return;
		}
		while ((Row = mysql_fetch_row(Res))) {
			index = pmyListCtrl.InsertItem(0,Row[8]);//ID
			pmyListCtrl.SetItemText(index,1,Row[3]); //To
			pmyListCtrl.SetItemText(index,2,DecodeDateTime(Row[11]));//Time2sent
			if (!strcmp(Row[9],"false")) {
				pmyListCtrl.SetItemText(index,3,"1");	 //parts
			} else {
				counter = 1;
				strcpy(something,outbox_multi);
				sprintf(something+strlen(something),"%s",Row[8]);
				if (mysql_real_query(GetDocument()->DB,something,strlen(something))) {
					MessageBox(mysql_error(GetDocument()->DB),"Query error");
					return;
				}
				if (!(Res = mysql_store_result(GetDocument()->DB))) {
					MessageBox(mysql_error(GetDocument()->DB),"Store error");
					return;
				}
				while ((Row = mysql_fetch_row(Res))) counter++;
				mysql_free_result(Res);
				sprintf(something,"%s",counter);
				pmyListCtrl.SetItemText(index,3,something);
			}
			pmyListCtrl.SetItemText(index,4,DecodeValidity(Row[10]));//validity
			pmyListCtrl.SetItemText(index,5,"a");	 //sending
			pmyListCtrl.SetItemText(index,6,Row[12]);//phone
			pmyListCtrl.SetItemText(index,7,Row[14]);//report
		}
		mysql_free_result(Res);
	}	
	if (pStr=="Sent items") {
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
		lvc.pszText = _T("Updated");
		lvc.cx = 125;
		pmyListCtrl.InsertColumn(3,&lvc);

		lvc.iSubItem = 4;
		lvc.pszText = _T("Phone");
		lvc.cx = 150;
		pmyListCtrl.InsertColumn(4,&lvc);

		if (mysql_real_query(GetDocument()->DB,sent,strlen(sent))) {
			MessageBox(mysql_error(GetDocument()->DB),"Query error");
			return;
		}
		if (!(Res = mysql_store_result(GetDocument()->DB))) {
			MessageBox(mysql_error(GetDocument()->DB),"Store error");
			return;
		}
		while ((Row = mysql_fetch_row(Res))) {
			index = pmyListCtrl.InsertItem(0,Row[11]);//ID
			pmyListCtrl.SetItemText(index,1,Row[5]);  //To
			pmyListCtrl.SetItemText(index,2,Row[8]);  //SMSC
			pmyListCtrl.SetItemText(index,3,DecodeDateTime(Row[0]));  //updated
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
	if (!m_wndSplitter2.CreateView(1, 0, RUNTIME_CLASS(CSplitEditView), CSize(0, 0), pContext)) 	 return FALSE;

	return TRUE;
}

void CChildFrame::ActivateFrame(int nCmdShow) 
{
	nCmdShow = SW_MAXIMIZE;   
	CMDIChildWnd::ActivateFrame(nCmdShow);
}

//---- main frame

static UINT BASED_CODE indicators[] =
{
	ID_SEPARATOR,
	ID_MODEM1,
	ID_MODEM2,
	ID_MODEM3,
	ID_MODEM4,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL
};
class CMainFrame:public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	void OnClickAbout() {
		CAboutDlg aboutDlg;
		aboutDlg.DoModal();
	}
	DECLARE_MESSAGE_MAP()
protected:
	CStatusBar StatusBar1;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct) {
		if (!CreateMutex(NULL, false, "GammuSMSDGatewayMutex") || GetLastError()!=0) {
			MessageBox("Application already run");
			return -1;
		}

		if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;

		StatusBar1.Create(this);
		StatusBar1.SetIndicators(indicators,sizeof(indicators)/sizeof(UINT));

		ASSERT((IDM_ABOUT & 0xFFF0) == IDM_ABOUT);
		ASSERT(IDM_ABOUT < 0xF000);

		CMenu* pSysMenu = GetSystemMenu(FALSE);
		if (pSysMenu != NULL) {
			CString strAboutMenu;
			strAboutMenu.LoadString(IDS_ABOUT);
			if (!strAboutMenu.IsEmpty()) {
				pSysMenu->AppendMenu(MF_SEPARATOR);
				pSysMenu->AppendMenu(MF_STRING, IDM_ABOUT, strAboutMenu);
			}
		}

		return 0;
	}
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus) {
		CMDIFrameWnd::OnShowWindow(bShow, nStatus);
	
		ShowWindow(SW_MAXIMIZE);
	}
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam) {
		if ((nID & 0xFFF0) == IDM_ABOUT) {
			OnClickAbout();
		} else {
			CMDIFrameWnd::OnSysCommand(nID, lParam);
		}
	}
};

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	ON_WM_SYSCOMMAND()
	ON_COMMAND(ID_HELP_ABOUT,OnClickAbout)
END_MESSAGE_MAP()

//-- application

class CMyApp:public CWinApp
{
public:
	BOOL InitInstance() {
		CMultiDocTemplate* 	pDocTemplate;
		CMDIFrameWnd* 		pMainFrame;

		setlocale(LC_ALL, ".ACP");

		InitCommonControls();

		pDocTemplate = new CMultiDocTemplate(IDR_MAIN,
			RUNTIME_CLASS(CMultiDoc),
			RUNTIME_CLASS(CChildFrame),
			RUNTIME_CLASS(CSplitTreeView));
		if (!pDocTemplate) return FALSE;
		AddDocTemplate(pDocTemplate);

		pMainFrame = new CMainFrame;
		if (pMainFrame==NULL) return FALSE;		
		if (!pMainFrame->LoadFrame(IDR_EMPTY)) return FALSE;
		pMainFrame->ShowWindow(m_nCmdShow);
		pMainFrame->UpdateWindow();
		m_pMainWnd = pMainFrame;

		MainHWND = m_pMainWnd->m_hWnd;

		mysql_init(&MainDB);

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
	void OnClickConfig() {
		CConfigDlg dlg;
		dlg.DoModal();
	}
	void OnClickPbk() {
		CEditPbkDlg dlg;
		dlg.DoModal();
	}
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CMyApp, CWinApp)
	ON_COMMAND(ID_FILE_EXIT,OnClickExit)
	ON_COMMAND(ID_FILE_NEW,OnClickNew)
	ON_COMMAND(ID_TOOLS_SENDSMS,OnClickSendSMS)
	ON_COMMAND(ID_TOOLS_CONFIG,OnClickConfig)
	ON_COMMAND(ID_TOOLS_PHONEBOOK,OnClickPbk)
END_MESSAGE_MAP()

CMyApp theApp;
