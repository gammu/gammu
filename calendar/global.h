#ifndef __DDR_GLOBAL_H_
#define __DDR_GLOBAL_H_

#include "..\Proj_Inc\ConvertEngine.h"
#include "include\skinheaderctrl.h"

#include <string>

using namespace std;
//#include "include\VSliderCtrlEx2.h"
//#include "panelctrl\ddr\AnwCommon20.h"
//#define WM_DROPPER_POS		(WM_USER+1)
#define WM_VIEWMODE_CHANGE		(WM_USER + 1001)

//DLL CONTRUCT
#define ID_EXIT_DLL_MESSAGE	    (WM_USER + 1100)

#define WM_TOOLBAR_DELSEL		(WM_USER + 2001)
#define WM_TOOLBAR_SEARCH		(WM_USER + 2002)
#define WM_TOOLBAR_NEWEDIT		(WM_USER + 2003)
#define WM_TOOLBAR_OPEN			(WM_USER + 2004)
#define WM_TOOLBAR_RELOAD		(WM_USER + 2005)
#define WM_TOOLBAR_BACKUP		(WM_USER + 2006)
#define WM_TOOLBAR_RESTORE		(WM_USER + 2007)

#define WM_SEARCH_STR			(WM_USER + 3001)
#define WM_SEARCH_END			(WM_USER + 3002)
#define WM_SEARCH_SETFLAG		(WM_USER + 3003)

#define WM_CARDPANEL_GETGROUP	(WM_USER + 4001)
#define WM_CARDPANEL_GROUPCOUNT	(WM_USER + 4002)

#define WM_LEFT_CHGMODE			(WM_USER + 5001)
#define WM_GETLEFTMODE			(WM_USER + 5002)
#define WM_ERR_OPENSYNC			(WM_USER + 5003)
#define WM_GETSYNCFLAG			(WM_USER + 5004)
#define WM_CHECKFILE			(WM_USER + 5005)
#define WM_LEFT_DROPDATA		(WM_USER + 5006)
#define WM_LOADMOBILE			(WM_USER + 5007) 
#define WM_SETLOADFILE			(WM_USER + 5008)
#define WM_SETLINKMOBILE		(WM_USER + 5009)
#define WM_OPENDRIVER			(WM_USER + 5010)
#define WM_INVALIDATEWND			(WM_USER + 5011)
#define WM_LEFT_DROPDATATOMSOT	(WM_USER + 5013)
#define WM_TOOLBAR_RELOAD_MSOT	(WM_USER + 5014)
#define WM_LEFT_UPDATEDATA	(WM_USER + 5015)

// ** Calendar ** //
//#define WM_OPENDRIVER			(WM_USER + 6001)//NONO
#define WM_MOBILECONNECTED		(WM_USER + 6002)//
#define WM_LOADCALENDAR	    	(WM_USER + 6003)//
#define WM_ADD_CALENDAR         (WM_USER + 6004)//
#define WM_OPENCALENDAR         (WM_USER + 6005)//
#define WM_REFRESH_LEFT_PANEL      (WM_USER + 6006)//
#define WM_SWITHTODAYVIEW      (WM_USER + 6007)//
#define WM_SETLEFTITEMCOUNT		(WM_USER + 6008)
#define WM_CHECK_DELETE		(WM_USER + 6009)
#define WM_CHECK_LEFTTREEITEM      (WM_USER + 6007)//

#define CTRL_EXT_CLASS
//define User Message
//tree use 
#define MOBILE_PHONE	_T("MOBILE_PHONE")
#define SIM_CARD		_T("SIM CARD")
#define MEMORY			_T("MEMORY")
#define COMPUTER		_T("COMPUTER")
#define CSVFILE			_T("CSVFILE")
#define MSOUTLOOK			_T("MSOUTLOOK")

//driver use
#define SIM_NAME		"SM"
#define MEMORY_NAME		"ME"

//define data type
#define DATALIST list<CCardPanel>

//define status
#define ST_MOBILE_LINK	0x0001
#define ST_FILE_LOAD	0X0002

#define  ANW_9a9u       WM_USER + 2007   //2007 1231 Kerm Added



typedef struct _FONTCOLOR
{
	COLORREF crNormal;
	COLORREF crDown;
	COLORREF crHover;	
	COLORREF crGray;

}FONTCOLOR;

typedef struct tagFileData{
	//store this item is which mode
	Tstring sMode;	
	//only for csv file mode
	int iIndex;	//store file index 
	Tstring sFile;	//store file full path 
	CString	sName;
	CString	sEntryID;
	CString	sStoreID;
	DWORD dwTypeFlag;	//FolderTypeFlag
//	DATALIST OTDataList;
	CPtrList CalendarDataList;
	bool     bLoad;
}FileData;

enum{
	OTHER = -1,
	NEW_CONTACT = 0,
	EDIT_CONTACT,
	VIEW_CONTACT
};


extern         int g_nMAX_SCHEDULE_COUNT;
extern         int g_nMAX_NOTES_COUNT;
extern         int g_nMAX_CONTACTS_COUNT;
extern       TCHAR gstrImageLocation[MAX_PATH];
extern         int g_numTotalCalendars;



// DLL CONTRUCT
CWnd * __stdcall afxGetMainWnd(void);
CWinApp * __stdcall afxGetApp(void);

void DrawShadow( Graphics& gdc, CRect& rect, int shadow, int factor=5, COLORREF color=RGB(64,64,64) );
void SystemError( DWORD dwError );
CString FileSize2Str(DWORD dwSize);
BOOL CenterRect(CRect *pRCParent,CRect *pRCChild,BOOL bOffset = FALSE);
void GetSliderData(SLDCTRLEX2_BUF &sld_data ,LPTSTR sSection ,LPTSTR sProfilePath);
int	 GetLargeStringLength(CString &str1 ,CString &str2,CDC *pDC);
void GetButtonFromSetting(CRescaleButton *pButton , TCHAR* sec , TCHAR* szTextID , TCHAR* szToolTip,TCHAR *profile );
void GetCheckFromSetting(CCheckEx2 *pCheck,TCHAR *sec,TCHAR *szTxtID,TCHAR *profile);
void GetPicFromSetting(CImage &Img,CRect &rc ,TCHAR *sec,TCHAR *profile);
void GetSliderFromSetting(CSliderCtrlEx2 *pSldCtrl,TCHAR* sec,TCHAR *profile);
void GetRadioFromSetting(CRadioEx *pRadio,TCHAR *sec,TCHAR *szTxtID,TCHAR *profile);
void GetStaticFromSetting(CStaticEx *pStatic,TCHAR *sec,TCHAR *szTxtID,TCHAR *profile);
bool GetProfileFont(TCHAR *profile, TCHAR *sec,int &nFontSize , TCHAR* szFontName);
bool GetProfileFont(TCHAR *profile, TCHAR *sec,TCHAR *sec2 ,int &nFontSize , TCHAR* szFontName);
HFONT GetFontEx(TCHAR *szFontName,int &nFontSize);
void DrawStringInImage(CImage *pImg, CString string ,CPoint pt,REAL FontSize,INT FontStyle,Color FontColor,int nDrawType,CSize EffectOffset,TCHAR*  pszFont );
void GetHeaderControlFromSetting(CSkinHeaderCtrl *pCtrl , TCHAR *sec , TCHAR *profile );
void GetCheckFromSetting(CCheckEx *pCheck, LPCTSTR szSec, LPCTSTR szText, LPCTSTR szToolTip, LPCTSTR szProfile);
void GetButtonFromSetting1(CRescaleButton *pButton, LPCTSTR szSec, LPCTSTR szText, LPCTSTR szToolTip, LPCTSTR szProfile);

bool nocase_compare(WCHAR c1,WCHAR c2);
void GetwhichWeekDay(GSM_DateTime gsmtime, UINT &whichWeek, UINT &whichDay);
void GetwhichWeekDay(COleDateTime dtOleTime, UINT &whichWeek, UINT &whichDay);

#endif //__DDR_GLOBAL_H_
