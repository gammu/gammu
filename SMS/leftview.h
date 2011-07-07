#if !defined(AFX_LEFTVIEW_H__9589190E_E96E_47FD_8823_1055CB8ABCB4__INCLUDED_)
#define AFX_LEFTVIEW_H__9589190E_E96E_47FD_8823_1055CB8ABCB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LeftView.h : header file
//

#pragma warning(disable:4786)
// item state flag
//



const int ciExpanded    = 1;
const int ciChecked     = 2;
const int ciDisabled    = 4;


// item styles
//
const int ciEditable    = 1;
const int ciCheckedNode = 2;
const int ciRadioNode   = 4;
const int ciSelectable  = 8;

#include <map>
using namespace std;

typedef struct _tagItemRects
{
  CRect m_oLeftRect;
  CRect m_oExpandRect;
  CRect m_oCheckRect;
  CRect m_oTextRect;
}
ITEM_RECTS, *P_ITEM_RECTS;


typedef map< HTREEITEM, P_ITEM_RECTS > ITEM_RECT_MAP;
typedef ITEM_RECT_MAP::iterator        ITEM_RECTS_ITER;


enum { bDot, bSolid };


//#include "oledroptarget.h"
#include "CPage.h"

//const int SMS_MOBILE_ItemCount = 300;//255;
const int SMS_MOBILE_ItemCount = 1000;//255;
const int SMS_ItemCount = 1000;//255;

/////////////////////////////////////////////////////////////////////////////
// CLeftView view
#define TREE_ITEM_NUM	9

class CLeftView : public CTreeView
{
protected:
	CLeftView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLeftView)

// Attributes
public:
	COleDropTarget	m_DropTarget;
	BOOL    m_bCapture;
	BOOL	m_bDisplayConnectStatus;
	SMS_Tal_Num m_sms_Tal_Num;
	bool   m_bRuningDlg;

// Operations
public:
	int			GetPCSMSInfo(void);
	int			GetAllSMSInfo(void);
	void		SetPCSaveSMSFolder(TCHAR *szBuffer);
	void		GetPCSaveSMSInfo(TCHAR *szBuffer);
	static	int	ConnectStatusCallBack(int State);
	int			GetMobileSMSInfo(void);
	static  int	GetMobileOneSMSInfo(int nNeed);
	static	int DeleteMobileOneSMS(int nNeed);
	static	int CopyMobileOneSMS(int nNeed);
	static	int MoveMobileOneSMS(int nNeed);
	static	int SendMobileOneSMS(int nNeed);
	void UpdateMobileConnectStatus(int State);

	void		SingleByteCut(CString lpszSrc, TCHAR* lpszDes, int nPage);
	void		MultiByteCut(CString lpszSrc, TCHAR* lpszDes, int nPage);

	int			(*fpSMSCallBack)(int x);
	int			xDownloadSMS(void);

	static		CLeftView* s_ActiveDlg;
	bool		m_bStart;
	bool		m_bFirst;
	bool		AsusNewTreeBuild;
	int			m_iMobilePos;
	bool		m_CtrlKeyDown;

	ASUS_GetData	asus_GetData[10];

	int			m_TreeItemBackup[255];	
	SMS_PARAM	*m_SIMSMS_Inbox, *m_SIMSMS_Outbox;
	SMS_PARAM	*m_HandsetSMS_Inbox, *m_HandsetSMS_Outbox;
	SMS_PARAM	*m_PC_Inbox, *m_PC_Outbox, *m_PC_SendBK;
	SMS_PARAM	*m_PC_Draft, *m_PC_Garbage;
	SMS_PARAM	*m_Mobile_SMS, *m_Mobile_DstSMS, m_Empty_box[10];
#ifdef ASUSM303
	SMS_PARAM	*m_HandsetSMS_Folder1, *m_HandsetSMS_Folder2, *m_HandsetSMS_Folder3;
	SMS_PARAM	*m_HandsetSMS_Folder4;//, *m_HandsetSMS_Folder5;
	GSM_SMSFolders mfolders;
#endif

	int			m_arrayDeletePosition[SMS_MOBILE_ItemCount];
	static		bool s_HasSIMCard;
	CStringList m_PrintPageList;
	//CDropTgt     m_droptarget;
	//IDropTargetHelper* m_piDropHelper;
   // bool               m_bUseDnDHelper;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLeftView)
	public:
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual void OnInitialUpdate();
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CLeftView();

	// Generated message map functions
protected:
	void		xAddToImageList(LPCTSTR lpszPath);
	CImageList  m_ImgList;
	Tstring		m_sProfile;
	HTREEITEM	m_hItemDrag;


	//{{AFX_MSG(CLeftView)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBeginrdrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnCustomDraw( NMHDR* pxNMHDR, LRESULT* plResult );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
//public:
	TCHAR		m_strPort[MAX_PATH], m_strPhone[MAX_PATH], m_strConnectMode[MAX_PATH];
	TCHAR		m_strMobileName[MAX_PATH], m_strCompany[MAX_PATH], m_strIMEI[MAX_PATH];
	TCHAR		m_strMobileDLL[MAX_PATH];
	TCHAR		m_strMobileName_ori[MAX_PATH];


	HMODULE		m_hMobileDll;
	HCURSOR		cursor_arr;
	HCURSOR		cursor_no;
	HFONT		m_hFont;

	int			GetMobileSetting(void);
	bool		InitialLibrary(void);
    void        FreeLibrary(HMODULE m_hDriver);
	int			OpenMobileConnect(void);
	void		UpdateAllTreeItem(int nPreSelItem, int nCurSelItem);
	void		UpdateOneTreeItem(HTREEITEM hCurSelItem, int nCurSelItem);
	void		UpdateAllTreeItemFirst(void);

	int			xGetPCSaveSMSInfo(CString szDir, SMS_PARAM *sms_info);
	int			xGetPCSaveSMSInfo_ANSCII(CString szDir);

	void		MoveItemsToPC(SMS_PARAM *SrcSMS, SMS_PARAM *DstSMS, int nPreSelItem, int nCurSelItem, int items);
	void		CopyItemsToPC(SMS_PARAM *SrcSMS, SMS_PARAM *DstSMS, int nPreSelItem, int nCurSelItem, int items);
	void		MoveItemsToMobile(SMS_PARAM *SrcSMS, SMS_PARAM *DstSMS, int nPreSelItem, int nCurSelItem, int items, short memtype);
	void		CopyItemsToMobile(SMS_PARAM *SrcSMS, SMS_PARAM *DstSMS, int nPreSelItem, int nCurSelItem, int items, short memtype);
	int			HasHighByte(CString pstrString);
	void		BackupAllDataToPC(void);
	void		RecursiveDelete(LPCTSTR pFoldername);
	void		 DeletePCSMSFile(LPCTSTR pFoldername);

	void		ProduceAFile(CString strPName,int nNextFile, SMS_PARAM smsInfo);

	void		PrintMessages(CPage* ps, SMS_PARAM *srcSMS, int nCount);

	void		DeletePCSMS(SMS_PARAM *srcSMS, int nCurSelItem);
	void		DeleteMobileSMS(SMS_PARAM *srcSMS, int nCurSelItem);
	void		DeleteMobileSMS_from_trash(SMS_PARAM *srcSMS, int nCurSelItem);

	void		qsort(SMS_PARAM input[], int first, int last, CString cDataType,bool bSortAscending);
	void		split(SMS_PARAM input[], int first, int last, int *split_point, CString cDataType,bool bSortAscending);
	void		swap(SMS_PARAM *src, SMS_PARAM *dst);

	void		xprintf(TCHAR *src, TCHAR *dst, int nNumber);

public:
	SMS_PARAM*	GetTreeSMSInfo(HTREEITEM hCurSelItem, int &nItems);

	void		xNewSMS();
	int 		xSendSMS(TCHAR *strGetMsg, CString PhoneNumberList, int nTotalPage, bool bMultiByte);
	int			xSaveSMSToDraftFolder(TCHAR *strGetMsg,CString PhoneNumberList, int nTotalPage, bool bMultiByte);
	int			xSaveSMSToDraftFolder(SMS_PARAM msgInfo, TCHAR *strPhoneList);
	int			xSaveSMSToBackUpFolder(SMS_PARAM msgInfo, TCHAR *strPhoneList);
	void		xSaveSMS(SMS_PARAM *srcSMS);
	void		xFilePrintSMS(void);
	void		xDeleteSMS(HTREEITEM hCurSelItem, SMS_PARAM *srcSMS);
	void		xRefreshSMS();
	void		xStartConnect();
	bool		XGetConnectStatus(TCHAR *str);
	void		xSortItem(CString cDataType,bool bSortAscending);
	int			GetSelectItem(HTREEITEM treeItem);



	void		CreateNewTreeList(void);
	void		SetFirstPosition(void);

private:
	CBrush		m_brush;
	DWORD		m_dwStyle;
	COLORREF	m_crBack, m_crText, m_crLine;
	UINT		m_nIndent;
	HICON		m_hIconBtn[2];
	HIMAGELIST	m_hImgList;

	CPoint		m_ptClick;



	void		PaintParentLine(HTREEITEM hParent, HDC hDC, RECT &rc);
	void		PaintItemLines(HTREEITEM hItem, HTREEITEM hParent, HDC hDC, RECT &rc);
	void		PaintButton(HDC hDC, RECT &rc, BOOL bExpanded);
	void		LineVert(HDC hDC, int x, int y0, int y1, bool nLine);
	void		LineHorz(HDC hDC, int x0, int x1, int y, bool nLine);


	void		xErrorHandling(int iErr);

};





/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEFTVIEW_H__9589190E_E96E_47FD_8823_1055CB8ABCB4__INCLUDED_)
