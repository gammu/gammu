//-----PhotoUtil-----------------------------------by karl--
#ifndef _MobilePhoto_
#define _MobilePhoto_
#include "FileSystem.h"

//typedef struct 
typedef struct
{
	int MEPhotoTotalNum;
	int MEPhotoUsedNum;
	int MEPhotoFolderFlag;
	int MEMMSFreeSpace;
	bool MemCardFlag;
	int MCPhotoTotalNum;
	int MCPhotoUsedNum;
	int MCMMSFreeSpace;
}MobilePhotoInfo;

typedef struct
{
	int Width_Boundary;
	int Height_Boundary;
	DWORD   PhotoFormat;
			// JPEG									0x00000001
			// BMAP									0x00000002
			// WBMP									0x00000004
			// GIf									0x00000080
			// PNG									0x00000010


}PhotoSupportInfo;


typedef enum {
	File_Image_JPG=1,
	File_Image_BMP,
	File_Image_WBMP,
	File_Image_GIF,
	File_Image_PNG,
}PhotoType; 


// Functions
int WINAPI OpenPhotoUtil(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI, int (*ConnectStatusCallBack)(int State) );
void WINAPI GetMobilePhotoInfo(MobilePhotoInfo * MobilePhotoInfo);
void WINAPI SetFileFolderPath(TCHAR *lpszFileString);
int WINAPI GetPhotoStarData(char *MemType , int PhotoType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[]);
int WINAPI GetPhotoNextData(char *MemType , int PhotoType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[]);
int WINAPI PutPhotoData(char *MemType , int PhotoType ,int *index, unsigned char *lpszFileString);
int WINAPI DeletePhotoData(char *MemType , int PhotoType , int index);
int WINAPI ClosePhotoUtil();
int WINAPI GetPhotoList(char *MemType , int PhotoType , int NeedCount, int *UsedNum, AnwMobile_FileInfo fileinfo[]);
int WINAPI ResetPhoto(void);
int WINAPI InitPhoto(void);
int WINAPI TerminatePhoto(void);
int WINAPI GetPhotoSupport(PhotoSupportInfo *SupportInfo);
int WINAPI SetFolderPath(char *folderpath);
int WINAPI GetPhotoFolderList(int *number, GSMFile *gsmFileList[]);
int WINAPI GetPhotoImageList(int *number, GSMFile *gsmFileList[]);
int WINAPI GetPhotoByFolder(char *SaveFolder, char *filename);
int WINAPI PutPhotoToFolder(char *FullPath_filename);
int WINAPI DeletePhotoByFolder(char *filename);

//-----PhotoUtil-----------------------------------by karl--
typedef int (WINAPI* OpenPhotoUtilProc)(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI , int (*ConnectStatusCallBack)(int State) );
typedef void (WINAPI* GetMobilePhotoInfoProc)(MobilePhotoInfo * MobilePhotoInfo);
typedef void (WINAPI* SetFileFolderPathProc)(unsigned char *lpszFileString);
typedef int (WINAPI* GetPhotoStarDataProc)(char *MemType , int PhotoType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[]);
typedef int (WINAPI* GetPhotoNextDataProc)(char *MemType , int PhotoType , int NeedCount , int *ReadCount, AnwMobile_FileInfo fileinfo[]);
typedef int (WINAPI* PutPhotoDataProc)(char *MemType , int PhotoType ,int *index, unsigned char *lpszFileString);
typedef int (WINAPI* DeletePhotoDataProc)(char *MemType , int PhotoType , int index);
typedef int (WINAPI* ClosePhotoUtilProc)();
typedef int (WINAPI* GetPhotoListProc)(char *MemType , int PhotoType , int NeedCount, int *UsedNum, AnwMobile_FileInfo fileinfo[]);
typedef int (WINAPI* ResetPhotoProc)(void);
typedef int (WINAPI* InitPhotoProc)(void);
typedef int (WINAPI* TerminatePhotoProc)(void);
typedef int (WINAPI* GetPhotoSupportProc)(PhotoSupportInfo *SupportInfo);
typedef int (WINAPI* SetFolderPathProc)(char *folderpath);
typedef int (WINAPI* GetPhotoFolderListProc)(int *number, GSMFile *gsmFileList[]);
typedef int (WINAPI* GetPhotoImageListProc)(int *number, GSMFile *gsmFileList[]);
typedef int (WINAPI* GetPhotoByFolderProc)(char *SaveFolder, char *filename);
typedef int (WINAPI* PutPhotoToFolderProc)(char *FullPath_filename);
typedef int (WINAPI* DeletePhotoByFolderProc)(char *filename);
//-----------------------------------------------------------

//-----PhotoUtil-----------------------------------by karl--
extern OpenPhotoUtilProc		OpenPhotoUtilfn;		
extern GetMobilePhotoInfoProc	GetMobilePhotoInfofn;
extern SetFileFolderPathProc	SetFileFolderPathfn;	
extern GetPhotoStarDataProc		GetPhotoStarDatafn;
extern GetPhotoNextDataProc		GetPhotoNextDatafn;		
extern PutPhotoDataProc			PutPhotoDatafn;
extern DeletePhotoDataProc		DeletePhotoDatafn;
extern ClosePhotoUtilProc		ClosePhotoUtilfn;
//extern GetPhotoListProc			GetPhotoListfn;
extern ResetPhotoProc			ResetPhotofn;
extern InitPhotoProc			InitPhotofn;
extern TerminatePhotoProc		TerminatePhotofn;
extern GetPhotoSupportProc		GetPhotoSupportfn;
extern SetFolderPathProc		SetFolderPathfn;
extern GetPhotoFolderListProc	GetPhotoFolderListfn;
extern GetPhotoImageListProc	GetPhotoImageListfn;
extern GetPhotoByFolderProc		GetPhotoByFolderfn;
extern PutPhotoToFolderProc		PutPhotoToFoldertfn;
extern DeletePhotoByFolderProc	DeletePhotoByFolderfn;

//-----------------------------------------------------------

#endif