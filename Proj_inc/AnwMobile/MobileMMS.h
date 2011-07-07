 //-----MMSUtil-----------------------------------by karl--
#ifndef _MobileMMS_
#define _MobileMMS_
#include "FileSystem.h"
#include "mbglobals.h"

typedef struct
{
	int		TotalNumber;
	int		MMSMaxSize;
	int     MMSMaxSlidesNum;
	int		PhotoMaxW;
	int		PhotoMaxH;
	int		PhotoMaxSize;
	int		RingTone;
	int		TextMaxBytes;
	int     SubjectMaxBytes;
	int		ToCCMaxBytes;
	int		MaxContacts;
	DWORD   PhotoFormat;
							//0x01	Photo_JPEG;
							//0x02  Photo_GIF;
							//0x04	Photo_WBMP;
							//0x08	Photo_BMP;
							//0x10	Photo_PNG;
	DWORD   RingToneFormat;
							//0x01	RingTone_MIDI;
							//0x02	RingTone_MMF;
							//0x04	RingTone_MP3;
							//0x08  RingTone_AMR;
							//0x10  RingTone_WAV;
	bool	TEXT_Unicode;
	bool	TEXT_UTF8;
	bool	TEXT_UTF16;
}MMSSupportInfo;

typedef struct
{
	int					Number;
	FileSystemFolder	Folder[10];
}MMSFolder;

//typedef struct 
typedef struct
{
	int MEMMSTotalNum;
	int MEMMSUsedNum;
	int MEMMSFolderFlag;
	int MEMMSFreeSpace;
	bool MemCardFlag;
	int MCMMSTotalNum;
	int MCMMSUsedNum;
	int MCMMSFreeSpace;
}MobileMMSInfo;

typedef enum {
	File_MMS_MMS=1,
}MMSType; 

// Functions
int WINAPI OpenMMSUtil(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI, int (*ConnectStatusCallBack)(int State) );
void WINAPI GetMobileMMSInfo(MobileMMSInfo * MobileMMSInfo);
void WINAPI SetMMSFileFolderPath(TCHAR *lpszFileString);
int WINAPI GetMMSStarData(char *MemType , int MMSType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[]);
int WINAPI GetMMSNextData(char *MemType , int MMSType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[]);
int WINAPI PutMMSData(char *MemType , int MMSType ,int *index, unsigned char *lpszFileString);
int WINAPI DeleteMMSData(char *MemType , int MMSType , int index);
int WINAPI CloseMMSUtil();
int WINAPI GetMMSList(char *MemType , int MMSType , int NeedCount, int *UsedNum, AnwMobile_FileInfo fileinfo[]);
int WINAPI ResetMMS(void);
int WINAPI GetMMSFolder(MMSFolder * mFolder);
int WINAPI InitMMS(void);
int WINAPI TerminateMMS(void);
int WINAPI GetMMSSupport(MMSSupportInfo *SupportInfo);
//New MMS
int WINAPI DelteMMSFile(GSM_File *file,int control);
int WINAPI AddMMSFile(GSM_File *ParentFile,GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal),int control);
int WINAPI GetMMSFile(GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal),int control);
int WINAPI GetMMSFileList(GSM_File *ParentFile, GSM_File *childFile,bool bStart);
int WINAPI InitMMSEx(void);
int WINAPI TerminateMMSEx(void);
int WINAPI OpenMMS(int MobileCompany , char *MobileID, char *ConnectMode,
		    char *ConnectPortName,char *IMEI, int (*ConnectStatusCallBack)(int State));
int WINAPI CloseMMS(void);

//-----MMSUtil-----------------------------------by karl--
typedef int (WINAPI* OpenMMSUtilProc)(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI, int (*ConnectStatusCallBack)(int State) );
typedef void (WINAPI* GetMobileMMSInfoProc)(MobileMMSInfo * MobileMMSInfo);
typedef void (WINAPI* SetMMSFileFolderPathProc)(TCHAR *lpszFileString);
typedef int (WINAPI* GetMMSStarDataProc)(char *MemType , int MMSType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[]);
typedef int (WINAPI* GetMMSNextDataProc)(char *MemType , int MMSType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[]);
typedef int (WINAPI* PutMMSDataProc)(char *MemType , int MMSType ,int *index, unsigned char *lpszFileString);
typedef int (WINAPI* DeleteMMSDataProc)(char *MemType , int MMSType , int index);
typedef int (WINAPI* CloseMMSUtilProc)();
typedef int (WINAPI* GetMMSListProc)(char *MemType , int MMSType , int NeedCount, int *UsedNum, AnwMobile_FileInfo fileinfo[]);
typedef int (WINAPI* ResetMMSProc)(void);
typedef int (WINAPI* GetMMSFolderProc)(MMSFolder * mFolder);
typedef int (WINAPI* InitMMSProc)(void);
typedef int (WINAPI* TerminateMMSProc)(void);
typedef int (WINAPI* GetMMSSupportProc)(MMSSupportInfo *SupportInfo);
//-----------------------------------------------------------
//New MMS
typedef int (WINAPI* DelteMMSFileProc)(GSM_File *file,int control);
typedef int (WINAPI* AddMMSFileProc)(GSM_File *ParentFile,GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal),int control);
typedef int (WINAPI* GetMMSFileProc)(GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal),int control);
typedef int (WINAPI* GetMMSFileListProc)(GSM_File *ParentFile, GSM_File *childFile,bool bStart);
typedef int (WINAPI* InitMMSExProc)(void);
typedef int (WINAPI* TerminateMMSExProc)(void);
typedef int (WINAPI* OpenMMSProc)(int MobileCompany , char *MobileID, char *ConnectMode,
		    char *ConnectPortName,char *IMEI, int (*ConnectStatusCallBack)(int State));
typedef int (WINAPI* CloseMMSProc)(void);

//-----MMSUtil-----------------------------------by karl--
extern OpenMMSUtilProc				OpenMMSUtilfn;		
extern GetMobileMMSInfoProc			GetMobileMMSInfofn;
extern SetMMSFileFolderPathProc		SetMMSFileFolderPathfn;	
extern GetMMSStarDataProc			GetMMSStarDatafn;
extern GetMMSNextDataProc			GetMMSNextDatafn;		
extern PutMMSDataProc				PutMMSDatafn;
extern DeleteMMSDataProc			DeleteMMSDatafn;
extern CloseMMSUtilProc				CloseMMSUtilfn;
extern GetMMSListProc				GetMMSListfn;
extern ResetMMSProc					ResetMMSfn;
extern GetMMSFolderProc				GetMMSFolderfn;
extern InitMMSProc					InitMMSfn;
extern TerminateMMSProc				TerminateMMSfn;
extern GetMMSSupportProc			GetMMSSupportfn;
//-----------------------------------------------------------

extern DelteMMSFileProc	DelteMMSFilefn;
extern AddMMSFileProc		AddMMSFilefn;
extern GetMMSFileProc		GetMMSFilefn;
extern GetMMSFileListProc	GetMMSFileLisfn;
extern InitMMSExProc		InitMMSExfn;
extern TerminateMMSExProc	TerminateMMSExfn;
extern OpenMMSProc			OpenMMSfn;
extern CloseMMSProc		CloseMMSfn;
#endif