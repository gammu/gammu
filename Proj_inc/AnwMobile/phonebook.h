#include <stdlib.h>



#define	CONNMODELIST	27
#define ERR_NONE		1

#ifndef _PHONEBOOK_
#define __PHONEBOOK__



#include "GSM_DATATYPE.h"
typedef enum
{
	ASUS_GROUP_All = 0,
	ASUS_GROUP_Friends,
	ASUS_GROUP_Family,
	ASUS_GROUP_Business,
	ASUS_GROUP_Colleagues,
	ASUS_GROUP_GROUP_VIP,
	ASUS_GROUP_Normal,
	ASUS_GROUP_Internet
} ASUS_GROUP;

typedef enum
{
	ASUS_SEX_None = 0,
	ASUS_SEX_Male,
	ASUS_SEX_Female
} ASUS_SEX;

typedef enum
{
	ASUS_LIGHT_Red = 0,
	ASUS_LIGHT_Blue,
	ASUS_LIGHT_Yellow,
	ASUS_LIGHT_Green,
	ASUS_LIGHT_Purple,
	ASUS_LIGHT_LightBlue,
	ASUS_LIGHT_BlueGreen,
	ASUS_LIGHT_DefaultColor = 255
} ASUS_LIGHT;
/*
#define MAX_INFO_SIZE  62
typedef struct
{
  PHONE_NUMBER normal_number;    // 一般電話
  PHONE_NUMBER Office_number;    // 公司電話
  PHONE_NUMBER mobile_number;    // 行動電話
  PHONE_NUMBER Home_number;      // 家用電話
  PHONE_NUMBER Home_number_1;    // 家用電話1 
  PHONE_NUMBER Office_number_1;  // 公司電話1
  PHONE_NUMBER BBCall;      
  PHONE_NUMBER Mobile_phone;     // 車用電話
  PHONE_NUMBER Office_fax;       // 公司傳真
  PHONE_NUMBER Home_fax;         // 家用傳真
  PHONE_NUMBER Other_fax;        // 其它傳真
  PHONE_NUMBER mobilephone_number;
  PHONE_NUMBER business_number;  // 商務電話
  PHONE_NUMBER business_number_1;// 商務電話1
  char FirstName[402];        // 姓名
  char LastName[402];         // 姓氏
  char Catalog[402];
  char Title[402];            // 職稱
  char email[402];            
  char email_1[402];
  char email_2[402];
  char HomePage[402];         //網頁位址
  char Division[402];         //部門
  char Company[402];          //公司
  char Office_street[402];    //公司街道
  char Office_city[402];      //公司城市
  char Office_state[402];     //公司縣市
  char Office_zip[402];       //公司區號
  char Home_street[402];      //家庭街道
  char Home_city[402];        //家庭城市
  char Home_state[402];       //家庭縣市
  char Home_zip[402];         //家庭區號
  char Country[402];          //國家
  char Note[402];             //備註
  char Postal_Address[402];   //全部地址
  char Couple[402];           //配偶
  char Son[402];              //小孩
  int  Rington;               //鈴聲
  int Group;                  //群組
  int Index;                  //索引
  //...add other member later. 
} Contact_Data_Struct;

*/
typedef struct
{
  int Total_Number;
  PHONE_BOUNDARY FirstName;        // 姓名
  PHONE_BOUNDARY LastName;         // 姓氏
  PHONE_BOUNDARY General_number;    // 一般電話
  PHONE_BOUNDARY Office_number;    // 公司電話
  PHONE_BOUNDARY mobile_number;    // 行動電話
  PHONE_BOUNDARY Home_number;      // 家用電話
  PHONE_BOUNDARY email;            
  PHONE_BOUNDARY Home_number_1;    // 家用電話1 
  PHONE_BOUNDARY Office_number_1;  // 公司電話1
  PHONE_BOUNDARY Catalog;
  PHONE_BOUNDARY BBCall;      
  PHONE_BOUNDARY Mobile_phone;     // 車用電話
  PHONE_BOUNDARY Office_fax;       // 公司傳真
  PHONE_BOUNDARY Home_fax;         // 家用傳真
  PHONE_BOUNDARY Other_fax;        // 其它傳真
  PHONE_BOUNDARY mobilephone_number;
  PHONE_BOUNDARY business_number;  // 商務電話
  PHONE_BOUNDARY business_number_1;// 商務電話1
  PHONE_BOUNDARY Title;            // 職稱
  PHONE_BOUNDARY email_1;
  PHONE_BOUNDARY email_2;
  PHONE_BOUNDARY HomePage;         //網頁位址
  PHONE_BOUNDARY Rington;               //鈴聲
  PHONE_BOUNDARY Division;         //部門
  PHONE_BOUNDARY Company;          //公司
  PHONE_BOUNDARY Postal_Address;	//住址
  PHONE_BOUNDARY Office_street;    //公司街道
  PHONE_BOUNDARY Office_city;      //公司城市
  PHONE_BOUNDARY Office_state;     //公司縣市
  PHONE_BOUNDARY Office_zip;       //公司區號
  PHONE_BOUNDARY Home_street;      //家庭街道
  PHONE_BOUNDARY Home_city;        //家庭城市
  PHONE_BOUNDARY Home_state;       //家庭縣市
  PHONE_BOUNDARY Home_zip;         //家庭區號
  PHONE_BOUNDARY Country;          //國家
  PHONE_BOUNDARY Note;             //備註
  PHONE_BOUNDARY Group;                  //群組
  //struct _timeb birthday;
  //struct _timeb Annal;
  PHONE_BOUNDARY Couple;           //配偶
  PHONE_BOUNDARY Son;              //小孩
  PHONE_BOUNDARY Reserved[10];
  //...add other member later. 
} Support_PhoneBook_Column;

typedef struct
{
  TCHAR IMEI[20];
  TCHAR manufacturer[50];
  TCHAR model[50];
  double fversion;
} MobileInfo;

/*typedef struct
{
	int MobileID;
	char *MobileName;
} MobileModel;*/


/*class node
{
public:

    int Index;
	int Location;
	bool flag;
    node* link;
};
*/


//--------------------------------------------------------------------------------
/*
#define MAX_MANUFACTURER_LENGTH		50
#define MAX_MODEL_LENGTH		50
#define MAX_VERSION_LENGTH		50
#define MAX_VERSION_DATE_LENGTH		50
#define	MAX_IMEI_LENGTH			20
#define PHONE_MAXSMSINFOLDER		200
#define GSM_MAX_NUMBER_LENGTH		50
#define GSM_MAXCALENDARTODONOTES	300
*/

//--------------------------------------------------------------------------------
int WINAPI GetPhoneBookSpace(char* MemoryType,
						  int* used,
						  int* free
						  );


int WINAPI DeletePhoneData(char* MemoryType, int Location,int Control);
  

//int WINAPI DeleteData(char* MemoryType, int Location);

int WINAPI AddPhoneBookData(char* MemoryType,				 
				   GSM_MemoryEntry*  GSM_Entry,
				   int Control);

int WINAPI GetPhoneBookStartData(char* MemoryType,				
				   GSM_MemoryEntry* GSM_Entry
				   );

int WINAPI GetPhoneBookNextData(char *MemoryType,					   
					   GSM_MemoryEntry* GSM_Entry,
					   bool start
					   );


int WINAPI GetMobileInfo(MobileInfo* mobileinfo);



int WINAPI GetGroupName(int Location, TCHAR* GroupName);


int WINAPI GetPhoneBookInfo(Contact_Tal_Num* Contact_Tal_Num);
int WINAPI GetPhoneBookInfoEx(char* MemoryType,int* used ,int* free);


int WINAPI UpdatePhoneBookData(char* MemoryType,GSM_MemoryEntry* ContactData);



int WINAPI GetPhoneBookSupportColumn(GSM_Phone_Support_Entry *ColumnList);
BOOL WINAPI IsSupportPhonebookDirectSMS();


int WINAPI GetAllGroup(TCHAR* GroupList[]);
int WINAPI InitPhoneBook(void);
int WINAPI TerminatePhoneBook(void);

//-----------------------------------------------------------
typedef int (WINAPI* OpenPhoneBookProc)(int MobileCompany , TCHAR *MobileID , TCHAR *ConnectMode , TCHAR *ConnectPortName ,TCHAR *IMEI, int (*ConnectStatusCallBack)(int State) );
typedef void (WINAPI* ClosePhoneBookProc)(void);
/*typedef int (WINAPI* GetMobileInfoProc)(MobileInfo* mobileinfo);
typedef int (WINAPI* GetGroupNameProc)(int Location, char* GroupName);
typedef int (WINAPI* GetMobileNumProc)(int MobileCompany);
typedef int (WINAPI* AddDataProc)(char* MemoryType, Contact_Data_Struct* ContactData);
typedef int (WINAPI* DeleteDataProc)(char* MemoryType, int Location);
typedef int (WINAPI* GetAllGroupProc)(char* GroupList[]);
typedef int (WINAPI* GetPhoneBookSupportColumnProc)(Support_PhoneBook_Column* ColumnList);
typedef int (WINAPI* UpdateContactDataProc)(char* MemoryType,Contact_Data_Struct* ContactData);




//int WINAPI DeleteData(char* MemoryType, int Location);*/
typedef int (WINAPI* GetPhoneBookInfoExProc)(char* MemoryType,int* used ,int* free);
 
typedef int (WINAPI* GetPhoneBookInfoProc)(Contact_Tal_Num* Contact_Tal_Num);
typedef int (WINAPI* GetGroupNumProc)(int* Num);
typedef int  (WINAPI*  GetPhoneBookSpaceProc)(char* MemoryType,int* used,int* free);
typedef int  (WINAPI*DeletePhoneBookDataProc)(char* MemoryType, char* szLocation, int Control);/* Type_Start
																						      Type_Continune
																							  Type_End */
typedef int  (WINAPI*DeletePhoneBookDataExProc)(char* MemoryType, char* szLocation, int Control,char* pszOriIndex);

typedef int  (WINAPI*AddPhoneBookDataProc)(char* MemoryType,GSM_MemoryEntry*  GSM_Entry, int Control);/* Type_Start
																						      Type_Continune
																							  Type_End */

typedef int  (WINAPI*GetPhoneBookStartDataProc)(char* MemoryType,	GSM_MemoryEntry* GSM_Entry);

typedef int  (WINAPI*GetPhoneBookNextDataProc)(char *MemoryType,GSM_MemoryEntry* GSM_Entry,bool start);


typedef int  (WINAPI*GetMobileInfoProc)(MobileInfo* mobileinfo);



typedef int  (WINAPI*GetGroupNameProc)(int Location, char* GroupName);


typedef int  (WINAPI*GetMobilePhoneBookInfoProc)(Contact_Tal_Num* Contact_Tal_Num);


typedef int  (WINAPI*UpdatePhoneBookDataProc)(char* MemoryType,GSM_MemoryEntry* ContactData);


typedef BOOL (WINAPI* IsSupportPhonebookDirectSMSProc)();

typedef int  (WINAPI*GetPhoneBookSupportColumnProc)(GSM_Phone_Support_Entry *ColumnList);
typedef int  (WINAPI*anwGetContactOriIndex)(char* ,char*);

typedef int  (WINAPI*GetAllGroupProc)(TCHAR* GroupList[]);						   
typedef int (WINAPI* InitPhoneBookProc)(void);
typedef int (WINAPI* TerminatePhoneBookProc)(void);
typedef int (WINAPI* InitialMemoryStatusProc)(char *MemoryType, int *uesd , int *free);
typedef int (WINAPI* InitialMemoryStatusExProc)(char *MemoryType, int *uesd , int *free,int (*pGetStatusfn)(int nCur,int nTotal));

extern OpenPhoneBookProc			OpenPhoneBookfn;
extern ClosePhoneBookProc			ClosePhoneBookfn;
extern GetPhoneBookInfoProc			GetPhoneBookInfofn;
extern GetPhoneBookStartDataProc	GetPhoneBookStartDatafn;
extern GetPhoneBookNextDataProc		GetPhoneBookNextDatafn;
extern DeletePhoneBookDataProc		DeletePhoneBookDatafn;
extern GetGroupNameProc				GetGroupNamefn;
extern DeletePhoneBookDataExProc		DeletePhoneBookDataExfn;
extern GetPhoneBookInfoExProc			GetPhoneBookInfoExfn;
/*extern SetMemoryProc SetMemoryfn;
extern GetMemoryProc GetMemoryfn;
extern GetMobileInfoProc GetMobileInfofn;
extern GetNumericDataProc GetNumericDatafn;*/
//extern GetPhoneBookSpaceProc GetPhoneBookSpacefn;
extern GetGroupNumProc				GetGroupNumfn;
extern GetMobileNumProc				GetMobileNumfn;
extern GetMobileListProc			GetMobileListfn;
extern AddPhoneBookDataProc			AddPhoneBookDatafn;
extern GetAllGroupProc				GetAllGroupfn;
extern GetPhoneBookSupportColumnProc GetPhoneBookSupportColumnnfn;
extern UpdatePhoneBookDataProc		UpdatePhoneBookDatafn;
extern InitPhoneBookProc			InitPhoneBookfn;
extern TerminatePhoneBookProc		TerminatePhoneBookfn;
extern  InitialMemoryStatusProc    InitialMemoryStatusfn;
extern anwGetContactOriIndex   ANWGetContactOriIndex;
extern  InitialMemoryStatusExProc    InitialMemoryStatusExfn;
extern IsSupportPhonebookDirectSMSProc IsSupportPhonebookDirectSMSfn;
#endif








