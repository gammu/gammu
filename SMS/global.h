#ifndef __DDR_GLOBAL_H_
#define __DDR_GLOBAL_H_

//#include "include\VSliderCtrlEx2.h"
//#include "panelctrl\ddr\AnwCommon20.h"
//#define WM_DROPPER_POS		(WM_USER+1)

#include "ConvertEngine.h"

#include "SkinHeaderCtrl.h"

#define WM_VIEWMODE_CHANGE		(WM_USER + 1001)

// DLL CONTRUCT
#define ID_EXIT_DLL_MESSAGE	    (WM_USER + 1100)

#define WM_TOOLBAR_DELSEL		(WM_USER + 2001)
#define  WM_MOBILE_CONNECT		(WM_APP+2002)
#define  WM_START_CONNECT		(WM_APP+2003)

#define CTRL_EXT_CLASS


#define NOKIA	1
//#define ASUSM303	1

#define SINGLEBYTE		160
#define MULTIBYTE		70
#define MAX_CONTENT		1000


typedef enum 
{
	MB_NOKIA = 1,
	MB_ASUS
} _MobileComapny;

_MobileComapny	gMobileCompany;

enum { SMSRpely = 0, 
       SMSSend};

//define User Message
typedef struct _FONTCOLOR
{
	COLORREF crNormal;
	COLORREF crDown;
	COLORREF crHover;	
	COLORREF crGray;

}FONTCOLOR;

#define SMS_DEVICE_SEPARATE		100
enum { itNONE	= 0,
	   itMOBILE = 1, 
	   itPC,
	   
	   itMOBILE_SIMCARD = 11,
	   itMOBILE_HANDSET,

	   itMOBILE_SIMCARD_INBOX = 21,
	   itMOBILE_SIMCARD_OUTBOX,
	   itMOBILE_SIMCARD_SENDBK,
	   itMOBILE_SIMCARD_DRAFT,
	   itMOBILE_SIMCARD_GARBAGE,
	   itMOBILE_SIMCARD_OTHER1,
	   itMOBILE_SIMCARD_OTHER2,
	   itMOBILE_SIMCARD_OTHER3,


	   itMOBILE_HANDSET_INBOX = 41,
	   itMOBILE_HANDSET_OUTBOX,
	   itMOBILE_HANDSET_SENDBK,
	   itMOBILE_HANDSET_DRAFT,
	   itMOBILE_HANDSET_GARBAGE,
	   itMOBILE_HANDSET_OTHER1,
	   itMOBILE_HANDSET_OTHER2,
	   itMOBILE_HANDSET_OTHER3,
	   itMOBILE_HANDSET_OTHER4,
	   //itMOBILE_HANDSET_OTHER5,
	  
	   itPC_INBOX = 101,
	   itPC_OUTBOX,
	   itPC_SENDBK,
	   itPC_DRAFT,
	   itPC_GARBAGE,};


enum { mmSIMCARD = 0, 
       mmHANDSETMEM, 
	   mmOTHER};

typedef struct _tagContacts
{
	TCHAR		strName[255];
	TCHAR		strTel[42];
	int			nStoreDevice;
} ContactList;

typedef struct tag_ASUS_GetData
{
	int	MemType;
	int FolderType;
	int	Items;

} ASUS_GetData;



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

void GetHeaderControlFromSetting(CSkinHeaderCtrl *pCtrl , TCHAR *sec , TCHAR *profile );
void DrawShadow( Graphics& gdc, CRect& rect, int shadow, int factor=5, COLORREF color=RGB(64,64,64) );
void SystemError( DWORD dwError );
CString FileSize2Str(DWORD dwSize);
BOOL CenterRect(CRect *pRCParent,CRect *pRCChild,BOOL bOffset = FALSE);
void GetSliderData(SLDCTRLEX2_BUF &sld_data ,LPTSTR sSection ,LPTSTR sProfilePath);
int	 GetLargeStringLength(CString &str1 ,CString &str2,CDC *pDC);
void GetButtonFromSetting(CRescaleButton *pButton , TCHAR* sec , UINT nTextID , UINT nToolTip,TCHAR *profile );
void GetCheckFromSetting(CCheckEx2 *pCheck,TCHAR *sec,UINT nTextID,TCHAR *profile);
void GetPicFromSetting(CImage &Img,CRect &rc ,TCHAR *sec,TCHAR *profile);
void GetSliderFromSetting(CSliderCtrlEx2 *pSldCtrl,TCHAR* sec,TCHAR *profile);
void GetRadioFromSetting(CRadioEx *pRadio,TCHAR *sec,UINT nTextID,TCHAR *profile);
void GetStaticFromSetting(CStaticEx *pStatic,TCHAR *sec,UINT nTxtID,TCHAR *profile);
bool GetProfileFont(TCHAR *profile, TCHAR *sec,int &nFontSize , TCHAR* szFontName);
bool GetProfileFont(TCHAR *profile, TCHAR *sec,TCHAR *sec2 ,int &nFontSize , TCHAR* szFontName);
HFONT GetFontEx(TCHAR *szFontName,int &nFontSize);
void DrawStringInImage(CImage *pImg, CString string ,CPoint pt,REAL FontSize,INT FontStyle,Color FontColor,int nDrawType,CSize EffectOffset,TCHAR*  pszFont );

#endif //__DDR_GLOBAL_H_
