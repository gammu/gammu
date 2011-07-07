#ifndef _AnwMobile_
#define _AnwMobile_



#include "phonebook.h"

int WINAPI GetMobileNum (int MobileCompany );


bool WINAPI GetMobileList (int MobileCompany,
						   int ArrayNum,
						   MobileModel  *MobileList
						   );


int WINAPI OpenSyncDrv(int MobileCompany,
					   int MobileID,
					   char *ConnectMode,
					   int ConnectPortNum,
					   int (*ConnectStatusCallBack)(int State)
					   );


void WINAPI CloseSyncDrv( );


int WINAPI GetMobileSpace(char* MemoryType,
						  int* used,
						  int* free
						  );


int WINAPI DeleteData(char* MemoryType, int Location);
  

int WINAPI AddData(char* MemoryType,				  
				   Contact_Data_Struct* ContactData
				   );


int WINAPI GetData(char* MemoryType,				  
				   Contact_Data_Struct* ContactData
				   );


int WINAPI GetNextData(char *MemoryType,					  
					   Contact_Data_Struct* ContactData,
					   bool start
					   );


int WINAPI GetMobileInfo(MobileInfo* mobileinfo);



int WINAPI GetGroupName(int Location, char* GroupName);


int WINAPI GetMobileContactInfo(Contact_Tal_Num* Contact_Tal_Num);


int WINAPI UpdateContactData(char* MemoryType,ContactData *pOldContactData,ContactData *pNewContactData);


int WINAPI GetNumericData(char* MemoryType,						 
						  Contact_Data_Struct* ContactDataList[],
						  int Num,
						  bool Reset
						  );


int WINAPI GetSupportColumn(int ColumnList[],char* ModelType,char* Path);


int WINAPI GetAllGroup(char* GroupList[]);

int WINAPI GetConnectModelList(char* ConnectModeList[]);

int WINAPI GetGroupNum(int* Num);

int WINAPI Entry2ContactData(GSM_MemoryEntry* Entry,Contact_Data_Struct* ContactData);

int WINAPI ContactData2Entry(GSM_MemoryEntry* Entry,Contact_Data_Struct* ContactData);

int WINAPI ResetMobile(void);

int WINAPI GetRefernceTable(const node* frontME, const node* frontSIM);

#endif	// _AnwMobile_
