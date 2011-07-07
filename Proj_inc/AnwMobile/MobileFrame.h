// Frame.h


#ifndef _MobileFrame_
#define _MobileFrame_
#include "GSM_DATATYPE.h"

#include "FileSystem.h"

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


}FrameSupportInfo;



// Functions
int	WINAPI OpenFrameUtil(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI, int (*ConnectStatusCallBack)(int State) );
void WINAPI SetFrameFileFolderPath(unsigned char *lpszFileString);
int WINAPI CloseFrameUtil();
int WINAPI InitFrame(void);
int WINAPI TerminateFrame(void);
int WINAPI GetFrameSupport(FrameSupportInfo *SupportInfo);
int WINAPI GetFrameImageList(int *number, GSMFile *gsmFileList[]);
int WINAPI GetFrame(char *SaveFolder, char *filename);
int WINAPI PutFrame(char *FullPath_filename);
int WINAPI DeleteFrame(char *filename);

//-----PhotoUtil-----------------------------------by karl--
typedef int (WINAPI* OpenFrameUtilProc)(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI , int (*ConnectStatusCallBack)(int State) );
typedef void (WINAPI* SetFrameFileFolderPathProc)(unsigned char *lpszFileString);
typedef int (WINAPI* CloseFrameUtilProc)();
typedef int (WINAPI* InitFrameProc)(void);
typedef int (WINAPI* TerminateFrameProc)(void);
typedef int (WINAPI* GetFrameSupportProc)(FrameSupportInfo *SupportInfo);
typedef int (WINAPI* GetFrameImageListProc)(int *number, GSMFile *gsmFileList[]);
typedef int (WINAPI* GetFrameProc)(char *SaveFolder, char *filename);
typedef int (WINAPI* PutFrameProc)(char *FullPath_filename);
typedef int (WINAPI* DeleteFrameProc)(char *filename);
//-----------------------------------------------------------

//-----PhotoUtil-----------------------------------by karl--
extern OpenFrameUtilProc			OpenFrameUtilfn;		
extern SetFrameFileFolderPathProc	SetFrameFileFolderPathfn;	
extern CloseFrameUtilProc			CloseFrameUtilfn;
extern InitFrameProc				InitFramefn;
extern TerminateFrameProc			TerminateFramefn;
extern GetFrameSupportProc			GetFrameSupportfn;
extern GetFrameImageListProc		GetFrameImageListfn;
extern GetFrameProc					GetFramefn;
extern PutFrameProc					PutFramefn;
extern DeleteFrameProc				DeleteFramefn;




#endif