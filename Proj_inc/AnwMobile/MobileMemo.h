// memo.h

#ifndef _MobileMemo_
#define _MobileMemo_
#include "GSM_DATATYPE.h"


typedef struct
{
	char Title[514];
	char Text[514];
	int Index;	 
} Memo_Data_Struct;


typedef struct 
{
	short  title;		// HighByte : Size limitation ; Low byte Mobile On :=0x01 ( bit flag )
	short  starttime;	// HighByte : Size limitation ; Low byte Mobile On :=0x01 ; ( bit flag )
	short  endtime;
} MemoStrc_Flag;

typedef struct
{
	

} MobilMemoStrc;

typedef struct
{ 
  short MemoUsedNum;
  short MemoTotalNum;
} Memo_Tal_Num;

typedef struct
{
										
}MemoSupportInfo;

//#define SchedulesRecord  Calendar_Data_Struct 



int WINAPI OpenMemo(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI, int (*ConnectStatusCallBack)(int State) );
int WINAPI GetMobileMemoStrc(MobilMemoStrc  *MobilMemoFlag);
int WINAPI GetMobileMemoInfo(Memo_Tal_Num *Memo_Tal_Num);
int WINAPI GetMemoStartData(Memo_Data_Struct *Memo_Data_Strc);
int WINAPI GetMemoNextData(Memo_Data_Struct *Memo_Data_Strc, bool bStart);
int WINAPI AddMemoData(Memo_Data_Struct *Memo_Data_Strc);
int WINAPI UpdateMemoData(Memo_Data_Struct *Memo_Data_Strc);
int WINAPI DeleteMemoData(int Index);
int WINAPI DeleteAllMemoData(Memo_Data_Struct *Memo_Data_Strc);
int WINAPI CloseMemo(void);
int WINAPI InitMemo(void);
int WINAPI TerminateMemo(void);
int WINAPI GetMemoSupport(MemoSupportInfo *SupportInfo);

typedef int (WINAPI* anwOpenMemo)(int MobileCompany , char *MobileID , char *ConnectMode , char *ConnectPortName ,char *IMEI, int (*ConnectStatusCallBack)(int State) );
typedef int (WINAPI* anwCloseMemo)(void);
typedef int (WINAPI* anwGetMobileMemoStrc)(MobilMemoStrc  *MobilMemoFlag);
typedef int (WINAPI* anwGetMobileMemoInfo)(Memo_Tal_Num *Memo_Tal_Num);
typedef int (WINAPI* anwGetMemoStartData)(Memo_Data_Struct *Memo_Data_Strc);
typedef int (WINAPI* anwGetMemoNextData)(Memo_Data_Struct *Memo_Data_Strc, bool bStart);
typedef int (WINAPI* anwAddMemoData)(Memo_Data_Struct *Memo_Data_Strc);
typedef int (WINAPI* anwUpdateMemoData)(Memo_Data_Struct *Memo_Data_Strc);
typedef int (WINAPI* anwDeleteMemoData)(int Index);
typedef int (WINAPI* anwDeleteAllMemoData)(Memo_Data_Struct *Memo_Data_Str);
typedef int (WINAPI* anwInitMemo)(void);
typedef int (WINAPI* anwTerminateMemo)(void);
typedef int (WINAPI* anwGetMemoSupport)(MemoSupportInfo *SupportInfo);

extern anwOpenMemo				ANWOpenMemo;
extern anwCloseMemo				ANWCloseMemo;
extern anwGetMobileMemoStrc		ANWGetMobileMemoStrc;
extern anwGetMobileMemoInfo		ANWGetMobileMemoInfo;
extern anwGetMemoStartData		ANWGetMemoStartData;
extern anwGetMemoNextData		ANWGetMemoNextData;
extern anwAddMemoData			ANWAddMemoData;
extern anwUpdateMemoData		ANWUpdateMemoData;
extern anwDeleteMemoData		ANWDeleteMemoData;
extern anwDeleteAllMemoData		ANWDeleteAllMemoData;
extern anwInitMemo				ANWInitMemo;
extern anwTerminateMemo			ANWTerminateMemo;
extern anwGetMemoSupport		ANWGetMemoSupport;
#endif