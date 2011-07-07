#ifndef __DDR_GLOBAL_H_
#define __DDR_GLOBAL_H_

#include "ConvertEngine.h"
#include "include\skinheaderctrl.h"
#include "Data Structure\CardPanel.h"
#include "MSOTDll.h"
#include <string>

using namespace std;

//defien mmobile,default is ASUS
#define ASUS_MOBILE 1

//#include "include\VSliderCtrlEx2.h"
//#include "panelctrl\ddr\AnwCommon20.h"
//#define WM_DROPPER_POS		(WM_USER+1)
#define WM_VIEWMODE_CHANGE		(WM_USER + 1001)
// DLL CONTRUCT
#define ID_EXIT_DLL_MESSAGE	    (WM_USER + 1100)

#define WM_TOOLBAR_DELSEL		(WM_USER + 2001)
#define WM_TOOLBAR_SEARCH		(WM_USER + 2002)
#define WM_TOOLBAR_NEWEDIT		(WM_USER + 2003)
#define WM_TOOLBAR_OPEN			(WM_USER + 2004)
#define WM_TOOLBAR_RELOAD		(WM_USER + 2005)
#define WM_TOOLBAR_BACKUP		(WM_USER + 2006)
#define WM_TOOLBAR_RESTORE		(WM_USER + 2007)
#define WM_TOOLBAR_SENDSMS		(WM_USER + 2008)

#define WM_SEARCH_STR			(WM_USER + 3001)
#define WM_SEARCH_END			(WM_USER + 3002)
#define WM_SEARCH_SETFLAG		(WM_USER + 3003)
#define WM_SEARCH_GETSTART		(WM_USER + 3004)

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
//#define WM_GETLISTSIZE			(WM_USER + 5011)
#define WM_DOWNLOAD_FAIL		(WM_USER + 5012)	
#define WM_LEFT_DROPDATATOMSOT	(WM_USER + 5013)
#define WM_TOOLBAR_RELOAD_MSOT	(WM_USER + 5014)
#define WM_LOAD_MSOTTREE			(WM_USER + 5015)

#define WM_THREADEND			(WM_USER + 6000)
#define WM_THREADBREAK			(WM_USER + 6001)
#define WM_PROGDLGSHOW			(WM_USER + 6002)
#define WM_SETLEFTITEMCOUNT		(WM_USER + 6003)
#define  WM_MOBILE_CONNECT		(WM_APP+6004)
#define  WM_CHECKLEFTTREEITEM		(WM_USER+6005)

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
#define CSVFILE_NAME		"CSVFILE"
#define MSOUTLOOK_NAME			"MSOUTLOOK"

//define data type
#define DATALIST list<CCardPanel>

//define status
#define ST_MOBILE_LINK		0x0001
#define ST_FILE_LOAD		0X0002
#define ST_DOWNLOAD_FAIL	0x0004
#define ST_WAIT_INIT		0x0008

//define the report size
#define REPORT_WIDTH		16
#define REPORT_HEIGHT		16


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
	DATALIST OTDataList;
	bool     bLoad;
}FileData;

enum{
	OTHER = -1,
	NEW_CONTACT = 0,
	EDIT_CONTACT,
	VIEW_CONTACT
};

enum{
	MOBILE_ID = 0,
	ME_ID = 1,
	SIM_ID = 2
};

enum{
	ENABLE_ME = 0,
	ENABLE_SIM = 1,
	ENABLE_VIEW
};

enum{
	CO_NOKIA = 1,
	CO_SONY_ERISON = 2,
	CO_SIMENS = 3,
	CO_ASUS = 4,
	CO_MOTO = 5,	
	CO_SAMSUNG = 6,		//bobby 8.3.05
	CO_SHARP = 7,		
	CO_PANASONIC = 8,		
	CO_LG = 9,		
	CO_SAGEM = 10,
	CO_TOSHIBA = 11,
	CO_MTK = 12
};
//SMS
#define SINGLEBYTE		160
#define MULTIBYTE		70
#define MAX_CONTENT		1000


typedef struct {
	TCHAR SCA[16];			// 短消息服務中心號碼(SMSC位址)
	TCHAR TPA[40];			// 目標號碼或回復號碼(TP-DA或TP-RA)
	char TP_PID;			// 用戶資訊協定標識(TP-PID)
	char TP_DCS;			// 用戶資訊編碼方式(TP-DCS)
	TCHAR TP_SCTS[40];		// 服務時間戳字串(TP_SCTS), 接收時用到
	TCHAR TP_UD[MAX_CONTENT+2];		// 原始用戶資訊(編碼前或解碼後的TP-UD)
	int  index;			// 短消息序號，在讀取時用到
	int	 whichFolder;		// 存在folder
	int  memType;			// 儲存mem
} SMS_PARAM;

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
void DrawStringInImage(CImage *pImg, CString string ,CRect &rcStr,REAL FontSize,
				INT FontStyle,Color FontColor,int nDrawType,CSize EffectOffset,TCHAR*  pszFont );
void MeasureStringRectangle(CImage *pImg,LPCTSTR lpStr,REAL FontSize,INT FontStyle,LPCTSTR lpFont,CRect &rcBound);

bool nocase_compare(WCHAR c1,WCHAR c2);
void CalcStaticRealRect(CStaticEx* pTxt, CRect& rect);

void SortList(DATALIST &lsData);
void SpriteAddress( int nCompany, LPCTSTR szAddress, LPTSTR szStreet, LPTSTR szCity , LPTSTR szState, LPTSTR szZIP, LPTSTR szCountry);
void MargeAddress( int nCompany, LPTSTR szAddress, LPCTSTR szStreet, LPCTSTR szCity, LPCTSTR szState , LPCTSTR szZIP, LPCTSTR szCountry);
void GetStaticFromSetting2(CStaticEx *pStatic,TCHAR *sec,TCHAR *szTxtID,TCHAR *profile);
void GetCheckFromSetting2(CCheckEx2 *pCheck,TCHAR *sec,TCHAR *szTxtID,TCHAR *profile);
void AnaString(LPTSTR szString,CStringList *pstrlist );
void FreeContactSubEntryList(Contact_Data_Struct* pData);
void MemoryEntry2ContactDataStruct(GSM_MemoryEntry* entry , Contact_Data_Struct* pContactData);
void ContactDataStruct2MemoryEntry(Contact_Data_Struct* pContactData,GSM_MemoryEntry* entry);
void MotoMargeAddress( LPTSTR szAddress,LPCTSTR szStreet2, LPCTSTR szStreet1, LPCTSTR szCity, LPCTSTR szState , LPCTSTR szZIP, LPCTSTR szCountry);
void MotoSpriteAddress( LPCTSTR szAddress, LPTSTR szStreet2, LPTSTR szStreet, LPTSTR szCity , LPTSTR szState, LPTSTR szZIP, LPTSTR szCountry);
void GetStaticFromSettingWithText(CStaticEx *pStatic,TCHAR *sec,TCHAR *szTxt,TCHAR *profile);
void	MultiByteCut(CString lpszSrc, TCHAR* lpszDes, int nPage);
void	SingleByteCut(CString lpszSrc, TCHAR* lpszDes, int nPage);
bool EntryTypeString2EntryType(CString szEntry,GSM_EntryType *pentryType);

//WCHAR* GetWharPhoneNumber(TCHAR *szPhoneNumber);
#endif //__DDR_GLOBAL_H_
