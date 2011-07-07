// MTK.h : main header file for the MTK DLL
//

#if !defined(AFX_MTK_H__02413EBC_270A_435E_9173_CB92F06DCF04__INCLUDED_)
#define AFX_MTK_H__02413EBC_270A_435E_9173_CB92F06DCF04__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "mbglobals.h"		// main symbols
#include "commfun.h"		// main symbols
#include "obexgenfun.h"		// main symbols
#include "AtGenFundef.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMTKApp
// See MTK.cpp for the implementation of this class
//
typedef struct {
	int			nMsgId; 
	unsigned char sz1[8];
	unsigned char szext[41];
	unsigned char szName[43];

} MMSFileInfo;
typedef struct {
	int			nMsgCount; 
	int			nUnread;
	unsigned char uHomedir[800];
	unsigned char uFileinfo[800];

} MMSFolderInfo;
// typedef  struct{  
// 	unsigned short  fileID; /*the ID of one phonebook record, if 0xffff indicate this item is empty, else there is a record with ID of fileID in phonebook. */   
// 	unsigned short phoneNumbersLastDigits[4];  /* not useful for us */
// 	char nameCheckSum[16];  /* not useful for us */ 
// 	char groupSet;    /* one byte padding */
// 	unsigned short    birthday; /* encode in BCD */
// 	unsigned short   reserved1; /* not use */
// 	unsigned char    reserved2; /* not use */
// }t_smpbRamInfo;
typedef  struct{  
   unsigned short  fileID; /*the ID of one phonebook record, if 0xffff indicate this item is empty, else there is a record with ID of fileID in phonebook. */   
  unsigned short phoneNumbersLastDigits[5];  /* not useful for us */
  char nameCheckSum[13];  /* not useful for us */ 
  char groupSet;    /* one byte padding */
}t_smpbRamInfo;

// typedef  struct{
// 	char BCDPhoneNb[22];  /* phone number, in BCD code */
// 	unsigned char  numberType; /* type of number, see enum definition below */
// 	unsigned char reserved; /* not use */
// }t_smartPhoneNumber;
typedef  struct{
   char BCDPhoneNb[22];  /* phone number, in BCD code */
   unsigned char  numberType; /* type of number, see enum definition below */
}t_smartPhoneNumber;


// typedef struct{
// 	char  name[61];
// 	t_smartPhoneNumber  phoneNumberSet[4];
// 	char    emailAddress [49]; /* E-mail */
// 	char   website[49]; /* personal web address */
// 	char    note [73]; /* note */
// 	char   address[91]; /* address */
// 	char   postcode[11]; /* postal code  */
// 	char   city[73]; /* from which city */
// 	char   province[73];  /* province */
// 	char   country[73];   /* country */
// 	char   birthday[11];  /* birthday in string format */
// 	char    picFileName[100];   /* big picture name */
// 	char    ringerFileName[100];   /* special ringer name */
// 	unsigned char groupType; /* the group that the record belong to.0: Default; 1:Family; 2: Friend; 3: Colleague; 4: Society; 5: Private */
// 	unsigned char defaultFieldType; /* default type of record, see enum definition below. */
// 	unsigned char  defaultNumberIndex; /* index of default number in phoneNumberSet */ 
// 	unsigned char  reserved; /* not use */
//}t_smartRecord;
typedef struct
{
char  name[61];
t_smartPhoneNumber  phoneNumberSet[5];
char    emailAddress [49]; /* E-mail */
char    note [73]; /* note */
char   address[91]; /* address */
char   postcode[11]; /* postal code  */
unsigned char groupType; /* the group that the record belong to.0: Default; 1:Family; 2: Friend; 3: Colleague; 4: Society; 5: Private */
unsigned char defaultFieldType; /* default type of record, see enum definition below. */
unsigned char  defaultNumberIndex; /* index of default number in phoneNumberSet */ 
}t_smartRecord;

typedef enum
{
	SMPB_TYPE_DEFAULT=0,	/* default */
	SMPB_TYPE_MOBILE,		/* mobile phone number */
	SMPB_TYPE_OFFICE,		/* office phone number */
	SMPB_TYPE_HOME,		/* home phone number */
	SMPB_TYPE_FAX,			/* fax number */
	SMPB_TYPE_PAGER,		/* pager number */  
	SMPB_TYPE_EMAIL,		/* e-mail address */
	/*SMPB_TYPE_NOTES,		/ * notes * /*/
	SMPB_TYPE_ADDRESS,		/* address */
	SMPB_TYPE_POSTCODE,	 /* postcode */
    SMPB_TYPE_NOTES,		 /* note book */
    SMPB_TYPE_NUMBER,		 /* phone number*/
    SMPB_TYPE_NOT_NUMBER,	/* not number */
    SMPB_TYPE_BIG_PICTURE,	/* big picture */
    SMPB_TYPE_NONE


} NXP_SMPB_TYPE;
///////////////////calendar080612libaoliu
typedef  struct 
{
   unsigned short	v_YearBitField:12;
   unsigned short	v_MonthBitField:4;
   unsigned short	v_DayBitField:5;
   unsigned short	v_HourBitField:5;
   unsigned short	v_MinutesBitField:6;
} t_gda_DateBitField;
typedef struct 
{
	unsigned char			v_Enable;
	unsigned char			v_RepeatType;
	unsigned short			v_Reserved1;
	t_gda_DateBitField		v_AlarmTime;	
	t_gda_DateBitField		v_AlarmTimeLeft;
}t_org_AlarmEvent;
typedef struct
{
	unsigned long			v_RecordNumber;
	t_org_AlarmEvent			a_Records[4];
}t_org_AlarmRecord;
typedef struct
{
	unsigned char			v_Other_Time;
	unsigned char			v_Enable;
	unsigned char			v_Other_Period;
	unsigned char			v_EventType;
	unsigned char			v_RepeatType;
	unsigned char			v_InformType;
	unsigned char			v_ReminderType;
	unsigned char			v_Reserved1;
	t_gda_DateBitField		v_StartTime;
	t_gda_DateBitField		v_EndTime;
	char					a_Text[48*3];
	char					a_Subject[(48*3)*3+1];
	char					a_Location[(48*3)*3+1];
}t_org_OrgEvent;
typedef struct 
{  
			unsigned long	v_RecordNumber;
			t_org_OrgEvent	a_Records[30];
}t_org_OrganizerRecord;


typedef struct 
{
			unsigned char	v_PowerOffTimeEnable;			/* no use */
			unsigned char	v_Reserved1;				/* no use */
			unsigned short	v_Reserved2;				/* no use */
			t_gda_DateBitField	v_PowerOffTime;			/* no use */
			t_org_OrganizerRecord	v_OrganizerRecords;		/* calendar */
			t_org_AlarmRecord	v_AlarmRecords;			/* no use */
}t_org_AllData;










class CMTKApp : public CWinApp
{
public:
	CMTKApp();

	bool m_bObexMode;
	Debug_Info	*m_pDebuginfo;
	GSM_ConnectionType *m_ConnectionType;
	OnePhoneModel *m_pMobileInfo;
	GSM_Error (*m_pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
								  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg));
	GSM_Error (* m_pSetATProtocolDatafn)(bool EditMode,bool bFastWrite,DWORD dwFlag);
	char m_szPHKTempFile[MAX_PATH*2];
	int m_PHKNum;
	OBEX_ConnectInfo m_ConnectInfo;
	GSM_File		*m_File;
	bool m_FileLastPart;
	//phonebook
	GSM_MemoryEntry	  m_PBKMemoryEntry;
	GSM_MemoryStatus *m_MemoryStatus;
	GSM_MemoryInfo m_MemoryInfo;
	GSM_AT_PBK_Charset m_PBKCharset;

	GSM_MemoryEntry*	  m_pContactEntry;
	//Calendar 
	int				m_LastCalendarPos;
	int				m_CalendarNum;
	int             m_nCalFirstInde;
	int             m_Caltotal_entries;
	int             m_CalendarUsedCount;
	int             m_nCalFirstIndex;
	char m_szCalendarTempFile[MAX_PATH*2];
	GSM_CalendarEntry m_CalendarEntry; // ???
	GSM_CalendarEntry	*m_CalNote;
	GSM_DateTime m_MobileDate_time;

	GSM_ToDoEntry  *m_Task;
	int				m_LastToDoPos;
	CString m_strCurrentFolderPath;

	//File
	bool         m_bSetPath;
	GSM_File	m_ChangeFolderPath;
	GSM_File			m_Files[2000];
	int				m_FilesLocationsUsed;
	int				m_FilesLocationsCurrent;
	GSM_Lines		m_Lines;
	int (*m_pGetFiletatusfn)(int nCur,int nTotal);
	int m_nPacketSize;
	bool			m_EditMode;

	DWORD  *m_nFreeSpace;
	//MMS
	CPtrList  m_MMSFileInfoList;
	int				m_FilesLocations[1000];
	MMSFolderInfo	m_MMSFolderInfo;
	//080424LIBAOLIU
	/*unsigned char *m_unEncodeBase64;*/
	unsigned char *m_unDecodeBase64;
	 char * position;
	char m_szPHKIndexFileTempFile[MAX_PATH*2];
	char m_szPHKDataFileTempFile[MAX_PATH*2];
	char m_szPHKAddDataFileTempFile[MAX_PATH*2];
	bool isFirst;
	bool isGetZero;
	bool isCancel;
	//080612libaoliu
	char m_szCalendarIndexFileTempFile[MAX_PATH*2];
	char m_szCalendarDataFileTempFile[MAX_PATH*2];
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMTKApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CMTKApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MTK_H__02413EBC_270A_435E_9173_CB92F06DCF04__INCLUDED_)
