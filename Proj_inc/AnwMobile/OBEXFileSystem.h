//-----Obex FileSystem-------------------------------------
#ifndef _OBEXFileSystem_
#define _OBEXFileSystem_
#include "FileSystem.h"

//------Bluetooth FTP AnWMobile Prototype-----------------------------by Bomber--
int WINAPI OpenObexSystemUtil(int MobileCompany ,TCHAR *MobileID ,TCHAR *ConnectMode ,TCHAR *ConnectPortName ,TCHAR *IMEI, int (*ConnectStatusCallBack)(int State));
int WINAPI InitObexSystem(void);
int WINAPI TerminateObexSystem(void);
//int WINAPI ObexSystemGetFolderList(int *number, GSM_File *GSM_FileList[]);
int WINAPI ObexSystemGetFileList(GSM_File FilePath,int *number, GSM_File *GSM_FileList[]);
int WINAPI ObexSystemGetFile(TCHAR *SaveFolder, GSM_File *filename, int Control,int (*pGetStatusfn)(int nCur,int nTotal));
int WINAPI ObexSystemPutFile(GSM_File FilePath,TCHAR *FullPath_filename,GSM_File* putFile, int Control,int (*pGetStatusfn)(int nCur,int nTotal));
int WINAPI ObexSystemDeleteFile(GSM_File FilePath,GSM_File *filename, int Control);
int WINAPI ObexSystemSetFolder(GSM_File FolderPath);//(char *folderpath);
int WINAPI CloseObexSystem(void);
//

/**
typedef int (WINAPI* OpenObexSystemUtilProc)(int MobileCompany ,char *MobileID ,char *ConnectMode ,char *ConnectPortName ,char *IMEI, int (*ConnectStatusCallBack)(int State));
typedef int (WINAPI* InitObexSystemProc)(void);
typedef int (WINAPI* TerminateObexSystemProc)(void);
//typedef int (WINAPI* ObexSystemGetFolderListProc)(int *number, GSM_File *GSM_FileList[]);
typedef int (WINAPI* ObexSystemGetFileListProc)(GSM_File FilePath,int *number, GSM_File *GSM_FileList[]);
typedef int (WINAPI* ObexSystemGetFileProc)(char *SaveFolder, char *filename);
typedef int (WINAPI* ObexSystemPutFileProc)(char *FullPath_filename);
typedef int (WINAPI* ObexSystemDeleteFileProc)(char *filename);
typedef int (WINAPI* ObexSystemSetFolderProc)(GSM_File FolderPath);//(char *folderpath);

//------Bluetooth FTP AnWMobile Export-----------------------------by Bomber--
extern OpenObexSystemUtilProc		OpenObexSystemUtilfn;
extern InitObexSystemProc			InitObexSystemfn;
extern TerminateObexSystemProc		TerminateObexSystemfn;
//extern ObexSystemGetFolderListProc	ObexSystemGetFolderListfn;
extern ObexSystemGetFileListProc	ObexSystemGetFileListfn;
extern ObexSystemGetFileProc		ObexSystemGetFilefn;
extern ObexSystemPutFileProc		ObexSystemPutFilefn;
extern ObexSystemDeleteFileProc	ObexSystemDeleteFilefn;
extern ObexSystemSetFolderProc		ObexSystemSetFolderfn;
//-----------------------------------------------------------
*/


#endif