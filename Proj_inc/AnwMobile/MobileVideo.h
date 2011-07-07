// Video.h

#ifndef _MobileVideo_
#define _MobileVideo_
#include "GSM_DATATYPE.h"

#include "FileSystem.h"



typedef struct
{
	int Width_Boundary; // no used
	int Height_Boundary;// no used
	DWORD   PhotoFormat;
			// 3GP									0x00000001
			// ASF									0x00000002
			// AVI									0x00000004
			// MOV									0x00000008
			// MPG									0x00000010
			// MPG4									0x00000020
			// WMV									0x00000040
			// rm									0x00000080
			// rmvb									0x00000100
}VideoSupportInfo;


typedef enum {
	/*File_Image_JPG=1,
	File_Image_BMP,
	File_Image_WBMP,
	File_Image_GIF,
	File_Image_PNG,*/
}VideoType; 


// Functions
int	WINAPI OpenVideoUtil(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI, int (*ConnectStatusCallBack)(int State) );
void WINAPI SetVideoFileFolderPath(unsigned char *lpszFileString);
int WINAPI CloseVideoUtil();
int WINAPI InitVideo(void);
int WINAPI TerminateVideo(void);
int WINAPI GetVideoSupport(VideoSupportInfo *SupportInfo);
int WINAPI SetVideoFolderPath(char *folderpath);
int WINAPI GetVideoFolderList(int *number, GSM_File *GSM_FileList[]);
int WINAPI GetVideoImageList(int *number, GSM_File *GSM_FileList[]);
int WINAPI GetVideo(char *SaveFolder, char *filename);
int WINAPI PutVideo(char *FullPath_filename);
int WINAPI DeleteVideo(char *filename);

//-----PhotoUtil-----------------------------------by karl--
typedef int (WINAPI* OpenVideoUtilProc)(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI , int (*ConnectStatusCallBack)(int State) );
typedef void (WINAPI* SetVideoFileFolderPathProc)(unsigned char *lpszFileString);
typedef int (WINAPI* CloseVideoUtilProc)();
typedef int (WINAPI* InitVideoProc)(void);
typedef int (WINAPI* TerminateVideoProc)(void);
typedef int (WINAPI* GetVideoSupportProc)(VideoSupportInfo *SupportInfo);
typedef int (WINAPI* SetVideoFolderPathProc)(char *folderpath);
typedef int (WINAPI* GetVideoFolderListProc)(int *number, GSM_File *GSM_FileList[]);
typedef int (WINAPI* GetVideoImageListProc)(int *number, GSM_File *GSM_FileList[]);
typedef int (WINAPI* GetVideoProc)(char *SaveFolder, char *filename);
typedef int (WINAPI* PutVideoProc)(char *FullPath_filename);
typedef int (WINAPI* DeleteVideoProc)(char *filename);
//-----------------------------------------------------------

//-----PhotoUtil-----------------------------------by karl--
extern OpenVideoUtilProc			OpenVideoUtilfn;		
extern SetVideoFileFolderPathProc	SetVideoFileFolderPathfn;	
extern CloseVideoUtilProc			CloseVideoUtilfn;
extern InitVideoProc				InitVideofn;
extern TerminateVideoProc			TerminateVideofn;
extern GetVideoSupportProc			GetVideoSupportfn;
extern SetVideoFolderPathProc		SetVideoFolderPathfn;
extern GetVideoFolderListProc		GetVideoFolderListfn;
extern GetVideoImageListProc		GetVideoImageListfn;
extern GetVideoProc					GetVideofn;
extern PutVideoProc					PutVideofn;
extern DeleteVideoProc				DeleteVideofn;
#endif