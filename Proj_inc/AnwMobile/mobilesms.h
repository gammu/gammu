#if !defined(AFX_ANWTEST_H__BE2FA6BC_B648_45AF_87EB_1FE5E7A382F8__INCLUDED_)
#define AFX_ANWTEST_H__BE2FA6BC_B648_45AF_87EB_1FE5E7A382F8__INCLUDED_


/*#define INBOX_FOLDER		0x0001
#define OUTBOX_FOLDER		0x0002
#define BACKUP_FOLDER		0x0004
#define OTHER1_FOLDER		0x0008
#define OTHER2_FOLDER		0x0010*/





typedef struct _SMS_Tal_Num
{
	short  SIMSMSTotalNum;
	short  SIMSMStUsedNum;			// -1 代表 N/A : Driver 無法取得
	short  MESMSTotalNum;
	short  MESMSUsedNum ;			// -1 代表 N/A : Driver 無法取得
	short  SIMSMSFolderTypeFlag;	// EX: 0x0000 = Send  ; 0x0001 = Receive ;
	short  MESMSFolderTypeFlag;		// EX: 0x0000 = Send  ; 0x0001 = Receive ;
	bool   bSIMSMSReadOnly;
	bool   bMESMSReadOnly;
} SMS_Tal_Num;

#define MAX_TP_UD	1000
typedef struct _SMS_Data_Strc
{
	TCHAR SCA[16];			// 短消息服務中心號碼(SMSC位址)
	TCHAR TPA[40];			// 目標號碼或回復號碼(TP-DA或TP-RA)
	char TP_PID;			// 用戶資訊協定標識(TP-PID)
	char TP_DCS;			// 用戶資訊編碼方式(TP-DCS)
	TCHAR TP_SCTS[40];		// 服務時間戳字串(TP_SCTS), 接收時用到
	TCHAR TP_UD[MAX_TP_UD+2];		// 原始用戶資訊(編碼前或解碼後的TP-UD)
	int  index;			// 短消息序號，在讀取時用到
	int  whichFolder;		// 存在folder
	int  memType;			// 儲存mem
} SMS_Data_Strc;




int WINAPI OpenSMS(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI, int (*ConnectStatusCallBack)(int State) );
int WINAPI GetMobileSMSInfo(SMS_Tal_Num *SMS_Tal_Num);

int WINAPI GetSMSStartData(int MemType, int NeedCount, SMS_Data_Strc *SMS_Data_Strc, int &RealCount); 
int WINAPI GetSMSOneData(int MemType, int FolderType, SMS_Data_Strc *SMS_Data_Strc, bool bStart);
int WINAPI GetSMSNextData (int MemType, int NeedCount, SMS_Data_Strc *SMS_Data_Strc, int &RealCount);
// Start : SyncDriver 從第一筆開始 read , 並 keep index 值 
// Next : SyncDriver 從第NeedCount+1筆開始 read , 並 keep index 值
// If RealCount < NeedCount , 代表讀完全部資料

int WINAPI SendSMSData(int MemType, int SMSfolderType, SMS_Data_Strc  * SMS_Data_Strc);
int WINAPI PutSMSData(int MemType, int SMSfolderType, SMS_Data_Strc  * SMS_Data_Strc);
int WINAPI DeleteSMSData(int MemType, int SMSfolderType, int Index);
// Note : 若手機不 Support delete , 由 SyncDriver 負責control , 不去 call delete // index 為 mobile 上的 index ;
int WINAPI DeleteAllSMSData(int MemType, int SMSfolderType);

int WINAPI CloseSMS(void);
int WINAPI InitSMS(void);
int WINAPI TerminateSMS(void);


// Export Function Define
typedef int (WINAPI* anwOpenSMS)(int MobileCompany , TCHAR *MobileID , TCHAR *ConnectMode , TCHAR *ConnectPortName ,TCHAR *IMEI, int (*ConnectStatusCallBack)(int State) );
typedef void (WINAPI* anwCloseSMS)(void);
typedef int (WINAPI* anwGetMobileSMSInfo)(SMS_Tal_Num *SMS_Tal_Num);
typedef int (WINAPI* anwGetMobileAllSMS)(SMS_Tal_Num *SMS_Tal_Num);
typedef int (WINAPI* anwGetSMSStartData)(int MemType, int NeedCount, SMS_Data_Strc *SMS_Data_Strc, int &RealCount);
typedef int (WINAPI* anwGetSMSNextData)(int MemType, int NeedCount, SMS_Data_Strc *SMS_Data_Strc, int &RealCount);
typedef int (WINAPI* anwSendSMSData)(int MemType, int SMSfolderType, SMS_Data_Strc  * SMS_Data_Strc);
typedef int (WINAPI* anwPutSMSData)(int MemType, int SMSfolderType, SMS_Data_Strc  * SMS_Data_Strc);
typedef int (WINAPI* anwDeleteSMSData)(int MemType, int SMSfolderType, int Index);
typedef int (WINAPI* anwDeleteAllSMSData)(int MemType, int SMSfolderType);

typedef int (WINAPI* anwGetSMSOneData)(int MemType, int FolderType, SMS_Data_Strc *SMS_Data_Strc, bool bStart);

typedef int (WINAPI* anwInitSMS)(void);
typedef int (WINAPI* anwTerminateSMS)(void);

extern anwOpenSMS				ANWOpenSMS;
extern anwCloseSMS				ANWCloseSMS;
extern anwGetMobileSMSInfo		ANWGetMobileSMSInfo;
extern anwGetMobileAllSMS		ANWGetMobileAllSMS;
extern anwGetSMSStartData		ANWGetSMSStartData;
extern anwGetSMSNextData		ANWGetSMSNextData;
extern anwSendSMSData			ANWSendSMSData;
extern anwPutSMSData			ANWPutSMSData;
extern anwDeleteSMSData			ANWDeleteSMSData;
extern anwDeleteAllSMSData		ANWDeleteAllSMSData;
extern anwInitSMS 				ANWInitSMSfn;
extern anwTerminateSMS 			ANWTerminateSMSfn;

#endif