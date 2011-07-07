//-----RingtoneUtil-----------------------------------by karl--
#ifndef _MobileRingtone_
#define _MobileRingtone_
#include "FileSystem.h"

//typedef struct 
typedef struct
{
	int MERingtoneTotalNum;
	int MERingtoneUsedNum;
	int MERingtoneFolderFlag;
	int MEMMSFreeSpace;
	bool MemCardFlag;
	int MCRingtoneTotalNum;
	int MCRingtoneUsedNum;
	int MCMMSFreeSpace;
}MobileRingtoneInfo;

typedef enum {
	File_Sound_MIDI=1,
	File_Sound_AMR,
	File_Sound_NRT,
}RingtoneType; 


typedef struct
{
	int Polyphonic;
	int NAME_LENGTH;
	int MaxFileSize;
	DWORD   RingtoneFormat;
			// MIDI									0x00000001
			// MMF									0x00000002
			// MP3									0x00000004
			// Mono									0x00000080
			// AMR									0x00000010
			// WAV									0x00000020
			// WMA									0x00000040
			// RA									0x00000080


}RingtoneSupportInfo;
// Functions
int WINAPI OpenRingtoneUtil(int MobileCompany , TCHAR *MobileID , TCHAR *ConnectMode , TCHAR *ConnectPortName ,TCHAR *IMEI, int (*ConnectStatusCallBack)(int State) );
void WINAPI GetMobileRingtoneInfo(MobileRingtoneInfo * MobileRingtoneInfo);
void WINAPI SetFileFolderPath(TCHAR *lpszFileString);
// Compare ; by mingfa
//int WINAPI GetRingtoneStarData(char *MemType , int RingtoneType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[]);
//int WINAPI GetRingtoneNextData(char *MemType , int RingtoneType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[]);
int WINAPI GetRingtoneStarData(char *MemType , int RingtoneType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[], BOOL bReal=TRUE);
int WINAPI GetRingtoneNextData(char *MemType , int RingtoneType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[], BOOL bReal=TRUE);
int WINAPI PutRingtoneData(char *MemType , int RingtoneType ,int *index, unsigned char *lpszFileString);
int WINAPI DeleteRingtoneData(char *MemType , int RingtoneType , int index);
int WINAPI CloseRingtoneUtil();
int WINAPI GetRingtoneList(char *MemType , int RingtoneType , int NeedCount, int *UsedNum, AnwMobile_FileInfo fileinfo[]);
int WINAPI ResetRingtone(void);
int WINAPI InitRingtone(void);
int WINAPI TerminateRingtone(void);
int WINAPI GetRingToneSupport(RingtoneSupportInfo *SupportInfo);
int WINAPI GetRingtoneFolderList(int *number, GSM_File *GSM_FileList[]);
int WINAPI GetRingToneImageList(int *number, GSM_File *GSM_FileList[]);
int WINAPI GetRingToneByFolder(char *SaveFolder, char *filename);
int WINAPI PutRingToneToFolder(char *FullPath_filename);
int WINAPI SetRingTonFolderPath(char *folderpath);
// Compare ; by Mingfa
int WINAPI GetRingToneByIndex(char *MemType , int PhotoType , int nIndex , AnwMobile_FileInfo &fileinfo);


//-----RingtoneUtil-----------------------------------by karl--
typedef int (WINAPI* OpenRingtoneUtilProc)(int MobileCompany , TCHAR *MobileID , TCHAR *ConnectMode , TCHAR *ConnectPortName ,TCHAR *IMEI, int (*ConnectStatusCallBack)(int State) );
typedef void (WINAPI* GetMobileRingtoneInfoProc)(MobileRingtoneInfo * MobileRingtoneInfo);
typedef void (WINAPI* SetRingtoneFileFolderPathProc)(TCHAR *lpszFileString);
typedef int (WINAPI* GetRingtoneStarDataProc)(char *MemType , int RingtoneType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[]);
typedef int (WINAPI* GetRingtoneNextDataProc)(char *MemType , int RingtoneType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[]);
typedef int (WINAPI* PutRingtoneDataProc)(char *MemType , int RingtoneType ,int *index, unsigned char *lpszFileString);
typedef int (WINAPI* DeleteRingtoneDataProc)(char *MemType , int RingtoneType , int index);
typedef int (WINAPI* CloseRingtoneUtilProc)();
typedef int (WINAPI* GetRingtoneListProc)(char *MemType , int RingtoneType , int NeedCount, int *UsedNum, AnwMobile_FileInfo fileinfo[]);
typedef int (WINAPI* ResetRingtoneProc)(void);
typedef int (WINAPI* InitRingtoneProc)(void);
typedef int (WINAPI* TerminateRingtoneProc)(void);
typedef int (WINAPI* GetRingToneSupportProc)(RingtoneSupportInfo *SupportInfo);
typedef int (WINAPI* GetRingtoneFolderListProc)(int *number, GSM_File *GSM_FileList[]);
typedef int (WINAPI* GetRingToneImageListProc)(int *number, GSM_File *GSM_FileList[]);
typedef int (WINAPI* GetRingToneByFolderProc)(char *SaveFolder, char *filename);
typedef int (WINAPI* PutRingToneToFolderProc)(char *FullPath_filename);
typedef int (WINAPI* SetRingTonFolderPathProc)(char *folderpath);
// Compare ; by Mingfa
//typedef int (WINAPI* DeleteRingtoneByFolderProc)(char *filename);
typedef int (WINAPI* GetRingToneByIndexProc)(char *MemType , int PhotoType , int nIndex , AnwMobile_FileInfo &fileinfo);
//-----------------------------------------------------------

//-----RingtoneUtil-----------------------------------by karl--
extern OpenRingtoneUtilProc				OpenRingtoneUtilfn;		
extern GetMobileRingtoneInfoProc		GetMobileRingtoneInfofn;
extern SetRingtoneFileFolderPathProc	SetRingtoneFileFolderPathfn;	
extern GetRingtoneStarDataProc			GetRingtoneStarDatafn;
extern GetRingtoneNextDataProc			GetRingtoneNextDatafn;		
extern PutRingtoneDataProc				PutRingtoneDatafn;
extern DeleteRingtoneDataProc			DeleteRingtoneDatafn;
extern CloseRingtoneUtilProc			CloseRingtoneUtilfn;
extern GetRingtoneListProc				GetRingtoneListfn;
extern ResetRingtoneProc				ResetRingtonefn;
extern InitRingtoneProc					InitRingtonefn;
extern TerminateRingtoneProc			TerminateRingtonefn;
extern GetRingToneSupportProc			GetRingToneSupportfn;
extern GetRingtoneFolderListProc		GetRingtoneFolderListfn;
extern GetRingToneImageListProc			GetRingToneImageListPfn;
extern GetRingToneByFolderProc			GetRingToneByFoldertfn;
extern PutRingToneToFolderProc			PutRingToneToFolderfn;
extern SetRingTonFolderPathProc			SetRingTonFolderPathfn;
// Compare ; by mingfa
//extern DeleteRingtoneByFolderProc		DeleteRingtoneByFolderfn;
extern GetRingToneByIndexProc			GetRingToneByIndexfn;
//-----------------------------------------------------------

#endif