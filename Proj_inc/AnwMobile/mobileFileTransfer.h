//-----PhotoUtil-----------------------------------by karl--
#ifndef _MobileFileTransfer_
#define _MobileFileTransfer_
#include "FileSystem.h"


int WINAPI OpenFileTransferUtil(int MobileCompany ,TCHAR *MobileID ,TCHAR *ConnectMode ,TCHAR *ConnectPortName ,TCHAR *IMEI, int (*ConnectStatusCallBack)(int State));
int WINAPI InitFileTransfer(void);
int WINAPI TerminateFileTransfer(void);

int WINAPI FileTransferGetFileList(GSM_File FilePath,int *number, GSM_File *GSM_FileList[]);
int WINAPI FileTransferGetFile(TCHAR *SaveFolder, GSM_File *filename , int Control);
int WINAPI FileTransferPutFile(GSM_File FilePath,TCHAR *FullPath_filename, int Control);
int WINAPI FileTransferDeleteFile(GSM_File FilePath,TCHAR *filename, int Control);
int WINAPI FileTransferGetFileEx(TCHAR *SaveFolder, GSM_File *filename , int Control,int (*pGetStatusfn)(int nCur,int nTotal));
int WINAPI FileTransferPutFileEx(GSM_File FilePath,TCHAR *FullPath_filename,GSM_File* putFile, int Control,int (*pGetStatusfn)(int nCur,int nTotal));


typedef int (WINAPI* OpenFileTransferUtilProc)(int MobileCompany ,TCHAR *MobileID ,TCHAR *ConnectMode ,TCHAR *ConnectPortName ,TCHAR *IMEI, int (*ConnectStatusCallBack)(int State));
typedef int (WINAPI* InitFileTransferProc)(void);
typedef int (WINAPI* TerminateFileTransferProc)(void);
typedef int (WINAPI* FileTransferGetFileListProc)(GSM_File FilePath,int *number, GSM_File *GSM_FileList[]);
typedef int (WINAPI* FileTransferGetFileProc)(TCHAR *SaveFolder, GSM_File *filename , int Control);
typedef int (WINAPI* FileTransferPutFileProc)(GSM_File FilePath,TCHAR *FullPath_filename,GSM_File* putFile, int Control);
typedef int (WINAPI* FileTransferDeleteFileProc)(GSM_File FilePath,GSM_File *filename, int Control);
typedef int (WINAPI* FileTransferGetFileExProc)(TCHAR *SaveFolder, GSM_File *filename , int Control,int (*pGetStatusfn)(int nCur,int nTotal));
typedef int (WINAPI* FileTransferPutFileExProc)(GSM_File FilePath,TCHAR *FullPath_filename,GSM_File* putFile, int Control,int (*pGetStatusfn)(int nCur,int nTotal));

extern OpenFileTransferUtilProc		OpenFileTransferUtilfn;
extern InitFileTransferProc			InitFileTransferfn;
extern TerminateFileTransferProc		TerminateFileTransferfn;
extern FileTransferGetFileListProc	FileTransferGetFileListfn;
extern FileTransferGetFileProc		FileTransferGetFilefn;
extern FileTransferPutFileProc		FileTransferPutFilefn;
extern FileTransferDeleteFileProc	FileTransferDeleteFilefn;
extern FileTransferPutFileExProc	FileTransferPutFileExfn;
extern FileTransferGetFileExProc	FileTransferGetFileExfn;

void GetGSMFilePath(GSM_File gsmfile,TCHAR *pFilePath);
void GetFileName(TCHAR* pFileFullName,TCHAR *pFileName);
void GetUnicodeFileName(GSM_File gsmfile,WCHAR *pFileName);
void GetGSMFileName(GSM_File gsmfile,TCHAR *pFileName);

#endif
