//-----PhotoUtil-----------------------------------by karl--
#ifndef _MobileBlueFTP_
#define _MobileBlueFTP_
#include "FileSystem.h"
//#include "typedef.h"

unsigned  UnicodeLength(const unsigned char *str);
void Unicode2Multi(WCHAR *lpszSrc, LPCSTR lpszDes);
void Multi2Unicode(LPCSTR lpszSrc, WCHAR *lpszDes);

//------Bluetooth FTP AnWMobile Prototype-----------------------------by Bomber--
int WINAPI OpenBlueFTPUtil(int MobileCompany ,TCHAR *MobileID ,TCHAR *ConnectMode ,TCHAR *ConnectPortName ,TCHAR *IMEI, int (*ConnectStatusCallBack)(int State));
int WINAPI InitBlueFTP(void);
int WINAPI TerminateBlueFTP(void);
int WINAPI BlueFTPGetFolderList(int *number, GSM_File *GSM_FileList[]);
int WINAPI BlueFTPGetFileList(int *number, GSM_File *GSM_FileList[]);
int WINAPI BlueFTPGetAllFileList(GSM_File FilePath,int *number, GSM_File *GSM_FileList[]);
int WINAPI BlueFTPGetFile(TCHAR *SaveFolder, TCHAR *filename);
int WINAPI BlueFTPPutFile(TCHAR *FullPath_filename);
int WINAPI BlueFTPDeleteFile(TCHAR *filename);
int WINAPI BlueFTPSetFolder(TCHAR *folderpath);
//

//------Bluetooth FTP AnWMobile Export-----------------------------by Bomber--
typedef int (WINAPI* OpenBlueFTPUtilProc)(int MobileCompany ,char *MobileID ,char *ConnectMode ,char *ConnectPortName ,char *IMEI, int (*ConnectStatusCallBack)(int State));
typedef int (WINAPI* InitBlueFTPProc)(void);
typedef int (WINAPI* TerminateBlueFTPProc)(void);
typedef int (WINAPI* BlueFTPGetFolderListProc)(int *number, GSM_File *GSM_FileList[]);
typedef int (WINAPI* BlueFTPGetFileListProc)(int *number, GSM_File *GSM_FileList[]);
typedef int (WINAPI* BlueFTPGetFileProc)(char *SaveFolder, char *filename);
typedef int (WINAPI* BlueFTPPutFileProc)(char *FullPath_filename);
typedef int (WINAPI* BlueFTPDeleteFileProc)(char *filename);
typedef int (WINAPI* BlueFTPSetFolderProc)(char *folderpath);

//------Bluetooth FTP AnWMobile Export-----------------------------by Bomber--
extern OpenBlueFTPUtilProc		OpenBlueFTPUtilfn;
extern InitBlueFTPProc			InitBlueFTPfn;
extern TerminateBlueFTPProc		TerminateBlueFTPfn;
extern BlueFTPGetFolderListProc	BlueFTPGetFolderListfn;
extern BlueFTPGetFileListProc	BlueFTPGetFileListfn;
extern BlueFTPGetFileProc		BlueFTPGetFilefn;
extern BlueFTPPutFileProc		BlueFTPPutFilefn;
extern BlueFTPDeleteFileProc	BlueFTPDeleteFilefn;
extern BlueFTPSetFolderProc		BlueFTPSetFolderfn;
//-----------------------------------------------------------

//------Bluetooth FTP Import-----------------------------by Bomber--
typedef GSM_Error (WINAPI* BTFTPInitProc)(void);
typedef GSM_Error (WINAPI* BTFTPTerminateProc)(void);
typedef GSM_Error (WINAPI* BTFTPGetFolderListProc)(GSM_File *GSM_FileList, long number);
typedef GSM_Error (WINAPI* BTFTPGetFileListProc)(GSM_File *GSM_FileList, long number);
typedef GSM_Error (WINAPI* BTFTPGetFileNumProc)(long* FileNum);
typedef GSM_Error (WINAPI* BTFTPGetFolderNumProc)(long* FolderNum);
typedef GSM_Error (WINAPI* BTFTPGetFileProc)(WCHAR *SaveFolder, WCHAR *filename);
typedef GSM_Error (WINAPI* BTFTPPutFileProc)(WCHAR *FullPath_filename);
typedef GSM_Error (WINAPI* BTFTPDeleteFileProc)(WCHAR *filename);
typedef GSM_Error (WINAPI* BTFTPSetFolderProc)(WCHAR *folderpath);
//-----------------------------------------------------------

//------Bluetooth FTP Import-----------------------------by Bomber--
static BTFTPInitProc			BTFTPInitfn;
static BTFTPTerminateProc		BTFTPTerminatefn;
static BTFTPGetFolderListProc	BTFTPGetFolderListfn;
static BTFTPGetFileListProc		BTFTPGetFileListfn;
static BTFTPGetFileNumProc		BTFTPGetFileNumfn;
static BTFTPGetFolderNumProc	BTFTPGetFolderNumfn;
static BTFTPGetFileProc			BTFTPGetFilefn;
static BTFTPPutFileProc			BTFTPPutFilefn;
static BTFTPDeleteFileProc		BTFTPDeleteFilefn;
static BTFTPSetFolderProc		BTFTPSetFolderfn;
//-----------------------------------------------------------

#endif
