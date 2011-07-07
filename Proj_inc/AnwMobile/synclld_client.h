
#ifndef __SYNCLLD_CLIENT_H_
#define __SYNCLLD_CLIENT_H_

#include "lld_interface.h"


typedef int (*LLD_TESTER_RUN)( unsigned int test ); 
typedef int (*test)(void);
typedef void (*lld_trace_printf)(WCHAR *format, ...);
typedef void (*lld_trace_printf_ansi)(char *format, ...);
typedef int (*LLD_TestPBRelay)( int MemType, int index );
typedef int (*LLD_TestPBWrite)( int MemType, WCHAR *filename );


//////////////////////////// LLD Interface //////////////////////////////////////////////////

//[LLD_FreeMemory]
//Description: used to free the memory that allocated by LLD library in advance, this is due to LLD and AP are at different heaps
//Parameters:
//	pMem: used to indicate the memory pointer that ask LLD to free
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_FreeMemory)( void *pMem );


//[LLD_SetConfig]
//Description: Used to set configuration to low level driver
//Parameters:
//	pConfig is used to store the prefer configuration setting for driver
//	Alloc/Free by caller
//	AP doesn't have to assign all settings in the structure, just some settings it interests and like to replace the default setting. 
//	The settings that AP not assigned will use the default setting.
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_SetConfig) (lld_Config *pConfig );


//[LLD_StartConnection]
//Description: Used to connect the mobile phone via the specific COM port
//Parameters:
//	Model is used to indicate the connect phone model
//	ComPort is used to indicate which com port to connect, referred to enum definition
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_PHONE_NOT_READY
//  LLD_RETURN_COMPORT_NOT_READY
//  LLD_RETURN_ALREADY_CONNECT
//********************************************************************
typedef int (*LLD_StartConnect)( char* model, char* ComPort, int (*ConnectStatus_CB)(int status) );


//[LLD_EndConnection]
//Description: Used to disconnect the mobile phone
//Parameters:
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_PHONE_NOT_READY
//	LLD_RETURN_COMPORT_NOT_READY
//	LLD_RETURN_ALREADY_DISCONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_EndConnect) ( void );


//[LLD_GetPhoneBookMemoryUsage]
//Description: Used to query for memory usage in specific memory
//Parameters:
//	MemType: used to indicate the target memory
//	pUsed: used to store the used file count
//		=> Alloc/Free by caller
//	pFree: used to store the unused file count
//		=> Alloc/Free by caller
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetPhoneBookMemoryUsage) ( int MemType, int* pUsed, int* pFree);

//[LLD_BrowserPhoneBookFolder]
//Description: Used to browse folder for target memory
//Parameters:
//	MemType: used to indicate the target memory
//	pList: used to stored the file index used in the folder
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int  (*LLD_BrowsePhoneBookFolder) ( int MemType, lld_FileIndexList *pList);


//[LLD_GetPhoneBookStartData]
//Description: Used to get first data
//Parameters:
//	MemType: used to indicate the target memory
//	pContactData: used to store the added data
//	Alloc/Free by caller
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//  LLD_RETURN_NO_DATA
//******************************************************************** 
typedef int (*LLD_GetPhoneBookStartData)( int MemType, lld_ContactData* pContactData );


//[LLD_GetPhoneBookNextData]
//Description: Used to get first data
//Parameters:
//	MemType: used to indicate the target memory
//	pContactData: used to store the added data
//		Alloc/Free by caller
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//	LLD_RETURN_NO_DATA
//******************************************************************** 
typedef int (*LLD_GetPhoneBookNextData)( int MemType, lld_ContactData* pContactData);


//[LLD_DeletePhoneBookData]
//Description: Used to delete the referred phone book data 
//Parameters:
//	MemType: used to indicate the target memory
//	Index: used to indicate the data index
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_DeletePhoneBookData)( int MemType, int Index);


//[LLD_AddPhoneBookData]
//Description: Used to delete the referred phone book data 
//Parameters:
//	MemType: used to indicate the target memory
//	pContactData: used to store the added data
//		=> Alloc/Free by caller
//		=> (Important) The stored index should be set in the structure in advanced by caller
//		=> Driver will depend on the dedicated index to store the data into the mobile phone, so AP have to make clear what index to store and what index is available
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int  (*LLD_AddPhoneBookData)( int MemType, lld_ContactData* pContactData );


//[LLD_GetPhoneBookDataByIndex]
//Description: Used to get all phone book data from the specific memory
//Parameters:
//	MemType: used to indicate the target memory
//	pContactData: used to store the specific data
//	=> Alloc/Free by caller
//Index: used to indicate the file index
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetPhoneBookDataByIndex)( int MemType, lld_ContactData* pContactData, int Index);


//[LLD_GetGroupNum]
//Description: used to get current group number in used
//Parameters:
//	pNum: used to store the returned number
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//  LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetGroupNum) (int *num);


//[LLD_GetAllGroup]
//Description: used to read all group information from phone
//Parameters:
//	pPhoneGroup: array for store the returned records
//		Alloc/Free by caller
//	n: indicate how many entities in the array
//Return:
//	Positive number: indicate the number of return records
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetAllGroup) ( lld_PhoneGroup* pPhoneGroup, int n );


//[LLD_GetCalendarNum]
//Description: used to read the number of calendar records in phone
//Parameters:
//Return:
//	Positive number: indicate the number of return records
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetCalendarNum) ( void );


//[LLD_BrowseCalendarFolder]
//Description: Used to browse folder for target memory
//Parameters:
//	pList: used to stored the file index used in the folder
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT_CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_BrowseCalendarFolder) ( lld_FileIndexList *pList);


//[LLD_GetCalendarStartData]
//Description: Used to get first data
//Parameters:
//	pCalendarData: used to store the added data
//	Alloc/Free by caller
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//	LLD_RETURN_NO_DATA
//******************************************************************** 
typedef int (*LLD_GetCalendarStartData)( lld_CalendarData* pCalendarData );


//[LLD_GetCalendarNextData]
//Description: Used to get first data
//Parameters:
//	pCalendarData: used to store the added data
//	Alloc/Free by caller
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//	LLD_RETURN_NO_DATA
//******************************************************************** 
typedef int (*LLD_GetCalendarNextData)( lld_CalendarData* pCalendarData );


//[LLD_GetCalendarDataByIndex]
//Description: used to read the specific calendar record
//Parameters:
//	pCalendarData: used to store the returned data
//	Index: used to indicate the file index of the record
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetCalendarDataByIndex) ( lld_CalendarData* pCalendarData, int Index );


//[LLD_AddCalendarData]
//Description: used to write the specific calendar record
//Parameters:
//	pCalendarData: used to store the returned data
//	Alloc/Free by caller
//	(Important) The stored index should be set in the structure in advanced by caller
//	Driver will depend on the dedicated index to store the data into the mobile phone, so AP have to make clear what index to store and what index is available
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_AddCalendarData) ( lld_CalendarData* pCalendarData );


//[LLD_DeleteCalendarData]
//Description: used to delete the specific calendar record
//Parameters:
//	Index: used to indicate the file index
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_DeleteCalendarData) ( int Index );


//[LLD_GetMemoNum]
//Description: used to read the number of memo records in phone
//Parameters:
//Return:
//	Positive number: indicate the number of return records
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetMemoNum) ( void );



//[LLD_BrowseMemoFolder]
//Description: Used to browse folder for target memory
//Parameters:
//	pList: used to stored the file index used in the folder
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_BrowseMemoFolder) ( lld_FileIndexList *pList);


//[LLD_GetMemoStartData]
//Description: Used to get first data
//Parameters:
//	pMemoData: used to store the added data
//		Alloc/Free by caller
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//	LLD_RETURN_NO_DATA
//******************************************************************** 
typedef int (*LLD_GetMemoStartData)( lld_MemoData* pMemoData );


//[LLD_GetMemoNextData]
//Description: Used to get first data
//Parameters:
//	pMemoData: used to store the added data
//		Alloc/Free by caller
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//	LLD_RETURN_NO_DATA
//******************************************************************** 
typedef int (*LLD_GetMemoNextData)( lld_MemoData* pMemoData );


//[LLD_GetMemoDataByIndex]
//Description: used to read the specific memo record
//Parameters:
//	pMemoData: used to store the returned data
//	Index: used to indicate the file index of the record
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetMemoDataByIndex) ( lld_MemoData* pMemoData, int Index );



//[LLD_AddMemoData]
//Description: used to write the specific memo record
//Parameters:
//	pMemoData: used to store the returned data
//	Alloc/Free by caller
//	(Important) The stored index should be set in the structure in advanced by caller
//	Driver will depend on the dedicated index to store the data into the mobile phone, so AP have to make clear what index to store and what index is available
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_AddMemoData) ( lld_MemoData* pMemoData );


//[LLD_DeleteMemoData]
//Description: used to delete the specific memo record
//Parameters:
//	Index: used to indicate the file index
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_DeleteMemoData) ( int Index );


//[LLD_GetSMSInfo]
//Description: used to acquire SMS information from phone
//Parameters:
//	pSMSInfo: used to store the returned data
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetSMSInfo) ( lld_SMSInfo *pSMSInfo );


//[LLD_GetSMSFolderName]
//Description: used to acquire SMS folder name from phone
//Parameters:
//	Folder: used to indicate the folder id
//	pFolderName: used to store the folder name
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetSMSFolderName) ( int MemType,  int folder, WCHAR **pFolderName );


//[LLD_BrowseSMSFolder]
//Description: Used to browse folder for target memory
//Parameters:
//	MemType: used to indicate the target memory
//	Folder:	used to indicate the target folder
//	pList: used to stored the file index used in the folder
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_BrowseSMSFolder) ( int MemType, int Folder, lld_FileIndexList *pList);

//[LLD_GetSMSStartData]
//Description: Used to get first data
//Parameters:
//	MemType: used to indicate the target memory
//	Folder: used to indicate the target folder
//	pTPDU: used to store the returned data
//		Alloc/Free by caller
//	len: used to indicate the length of pTPDU
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//	LLD_RETURN_NO_DATA
//******************************************************************** 
typedef int (*LLD_GetSMSStartData)( int MemType, int Folder, 
								  unsigned char *pTPDU, unsigned int *len);


//[LLD_GetSMSNextData]
//Description: Used to get first data
//Parameters:
//	MemType: used to indicate the target memory
//Folder: used to indicate the target folder
//	pTPDU: used to store the returned data
//		Alloc/Free by caller
//	len: used to indicate the length of pTPDU
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//	LLD_RETURN_NO_DATA
//******************************************************************** 
typedef int (*LLD_GetSMSNextData)( int MemType, int Folder, 
								 unsigned char *pTPDU, unsigned int *len);


//[LLD_GetSMSDataByIndex]
//Description: used to acquire the specific SMS data from specific folder and memory
//Parameters:
//	MemType: used to indicate the target memory
//	Folder:	used to indicate the target folder
//	Index: used to indicate the file index
//	pTPDU: used to store the returned data
//		=> Alloc/Free by caller
//	len: used to indicate the length of pTPDU
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetSMSDataByIndex) ( int MemType, int Folder,
								int index, unsigned char *pTPDU, unsigned int *len);


//[LLD_SendSMSData]
//Description: used to send SMS data
//Parameters:
//	PhoneNumber: used to indicate the target phone number
//	Content: used to store the SMS content
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//  LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_SendSMSData) ( WCHAR *PhoneNumber, WCHAR *content );


//[LLD_PutSMSData]
//Description: used to write SMS data into the specific folder and memory
//Parameters:
//	MemType: used to indicate the target memory
//	Index: used to indicate the file index
//	pTPDU: used to store the returned data
//			Alloc/Free by caller
//	len: used to indicate the length of pTPDU
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_PutSMSData) ( int MemType, int Folder, int index, 
								unsigned char *pTPDU, unsigned int len );


//[LLD_DeleteSMSData]
//Description: used to delete the specific SMS data into the specific folder and memory
//Parameters:
//	MemType: used to indicate the target memory
//	Folder: used to indicate the target folder
//	Index: used to indicate the file index
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_DeleteSMSData) ( int MemType, int Folder, int Index);


//[LLD_GetMMSInfo]
//Description: used to acquire MMS information from phone
//Parameters:
//	pMMSInfo: used to store the returned data
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetMMSInfo) ( lld_MMSInfo *pMMSInfo );


//[LLD_GetMMSFolderName]
//Description: used to acquire MMS folder name from phone
//Parameters:
//	Folder: used to indicate the folder id
//	pFolderName: used to store the folder name
//		=> Alloc by calle, free by caller
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetMMSFolderName) ( int folder, WCHAR **pFolderName);


//[LLD_BrowseMMSFolder]
//Description: Used to browse folder for target memory
//Parameters:
//	Folder:	used to indicate the target folder
//	pList: used to stored the file index used in the folder
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_BrowseMMSFolder) ( int Folder, lld_FileIndexList *pList);


//[LLD_GetMMSData]
//Description: used to write the specific MMS data into the specific file
//Parameters:
//	Folder: used to indicate the target folder
//	Index: used to indicate the file index
//	lpszFileString: used to indicate the file name
//	=> Be care of wide characters in path name
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetMMSData) ( int Folder, int index, LPTSTR lpszFileString );


//[LLD_GetMMSStartData]
//Description: Used to get first data
//Parameters:
//	MemType: used to indicate the target memory
//	Folder: used to indicate the target folder
//	lpszFileString: used to store the file name that store the data (in PC side)
//	=> Be care of wide characters in path name
//
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//	LLD_RETURN_NO_DATA
//******************************************************************** 
typedef int (*LLD_GetMMSStartData)( int MemType, int Folder, LPTSTR lpszFileString);


//[LLD_GetMMSNextData]
//Description: Used to get first data
//Parameters:
//	MemType: used to indicate the target memory
//Folder: used to indicate the target folder
//	lpszFileString: used to store the file name that store the data (in PC side)
//	=> Be care of wide characters in path name
//
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//	LLD_RETURN_NO_DATA
//******************************************************************** 
typedef int (*LLD_GetMMSNextData)( int MemType, int Folder, LPTSTR lpszFileString);


//[LLD_PutMMSData]
//Description: used to read the specific MMS data into the specific file
//Parameters:
//	Folder: used to indicate the target folder
//  index: used to indicate the file index
//	lpszFileString: used to store the file name
//	=> Be care of wide characters in path name
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_PutMMSData) ( int Folder, int index, LPTSTR lpszFileString );


//[LLD_DeleteMMSData]
//Description: used to delete the specific MMS data
//Parameters:
//	Folder: used to indicate the target folder
//	Index: used to indicate file index
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_DeleteMMSData) ( int Folder, int index );



//[LLD_GetImageGalleryInfo]
//Description: used to acquire ImageGallery information from phone
//Parameters:
//	pImageGalleryInfo: used to store the returned data
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetImageGalleryInfo) ( lld_ImageGalleryInfo *pImageGalleryInfo );


//[LLD_GetImageGalleryFileList]
//Description: used to acquire ImageGallery file name list from phone
//Parameters:
//	Folder: used to indicate the target folder
//	pFileList: used to store the returned data
//		Alloc/Free by caller
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetImageGalleryFileList) ( int folder, lld_FileList *pFileList );


//[LLD_SetImageGalleryFileFolderPath]
//Description: used to notify where to read and write Photo files by driver
//Parameters:
//	lpszFolderString: used to store the path name
//	=> Be care of wide characters in path name
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//  LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_SetImageGalleryFileFolderPath)( LPTSTR lpszFolderString );


//[LLD_GetImageGalleryData]
//Description: used to write the specific ImageGallery data into the specific file
//Parameters:
//	Folder: used to indicate the target folder
//	lpszFileString: used to indicate the file name
//	=> Be care of wide characters in path name
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetImageGalleryData) ( int Folder, LPTSTR lpszFileString );


//[LLD_PutPhotoData]
//Description: used to read the specific Photo data into the specific file
//Parameters:
//	Folder: used to indicate the target folder
//	lpszFileString: used to store the file name
//	=> Be care of wide characters in path name
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_PutImageGalleryData) ( int folder, LPTSTR lpszFileString );



///[LLD_DeletePhotoData]
//Description: used to delete the specific Image Gallery data
//Parameters:
//	Folder: used to indicate the target folder
//	lpszFileString: used to indicate the file name
//	=> Be care of wide characters in path name
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_DeleteImageGalleryData) ( int folder, LPTSTR lpszFileString );


//[LLD_GetRingToneInfo]
//Description: used to acquire Ring Tone information from phone
//Parameters:
//	pRingToneInfo: used to store the returned data
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetRingToneInfo) ( lld_RingToneInfo *pRingToneInfo );


//[LLD_GetRingToneFileList]
//Description: used to acquire Ring Tone file name list from phone
//Parameters:
//	Folder: used to indicate the target folder
//	pFileList: used to store the returned data
//		Alloc/Free by caller
//		filenameA=> alloc by callee, free by caller
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//********************************************************************
typedef int (*LLD_GetRingToneFileList) ( int Folder, lld_FileList *pFileList );


//[LLD_SetRingToneFileFolderPath]
//Description: used to notify where to read and write Ring Tone files by driver
//Parameters:
//	lpszFolderString: used to store the path name
//	=> Be care of wide characters in path name
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_SetRingToneFileFolderPath) ( LPTSTR lpszFolderString );


//[LLD_GetRingToneData]
//Description: used to write the specific Ring Tone data into the specific file
//Parameters:
//	Folder: used to indicate the target folder
//	lpszFileString: used to indicate the file name
//	=> Be care of wide characters in path name
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetRingToneData) ( int folder, LPTSTR lpszFileString );


//[LLD_PutRingToneData]
//Description: used to read the specific Ring Tone data into the specific file
//Parameters:
//	Folder: used to indicate the target folder
//	lpszFileString: used to store the file name
//	=> Be care of wide characters in path name
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_PutRingToneData) ( int Folder,	LPTSTR lpszFileString );


//[LLD_DeleteRingToneData]
//Description: used to delete the specific Ring Tone data
//Parameters:
//	Folder: used to indicate the target folder
//	lpszFileString: used to store the file name
//	=> Be care of wide characters in path name
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//********************************************************************
typedef int (*LLD_DeleteRingToneData) ( int folder,	LPTSTR lpszFileString );


//[LLD_Backup]
//Description: used to write backup data into the specific file
//Parameters:
//lpszFileString: used to indicate the file name
//=> Be care of wide characters in path name 
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_Backup) ( LPTSTR lpszFileString );


//[LLD_Restore]
//Description: used to get restore data from the specific file
//Parameters:
//lpszFileString: used to indicate the file name
//=> Be care of wide characters in path name 
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_Restore) ( LPTSTR lpszFileString );



//[LLD_GetJavaNum]
//Description: used to read the number of Java programs in phone
//Parameters:
//Return:
//	Positive number: indicate the number of 
//return records
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetJavaNum) ( void );


//[LLD_DownloadJava]
//Description: used to download Java to mobile phone
//Parameters:
//	FileJAD: used to indicate the filename of JAD (in PC side)
//	FileJAR: used to indicate the filename of JAR (in PC side)
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_DownloadJava) ( LPTSTR FileJAD, LPTSTR FileJAR );

//[LLD_GetJavaInfo]
//Description: used to acquire Java information from phone
//Parameters:
//	pJavaInfo: used to store the returned data
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetJavaInfo) ( lld_JavaInfo *pJavaInfo );

//[LLD_GetMobileInfo]
//Description: Used to acquire phone information, including IMEI, vendor, model and firmware version
//Parameters:
//	pMobileinfo is used to store model string
//		=> Alloc/Free by caller
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_GetMobileInfo) ( WCHAR *pMobileInfo );

//[LLD_SetupProgress]
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_SetupProgress)(int (*ProgressStatus_CB)(int percent) );


//[LLD_StartProgress]
//Return:
//	LLD_RETURN_OK
//	LLD_RETURN_ERROR
//	LLD_RETURN_NOT _CONNECT
//	LLD_RETURN_CALL_ERROR
//********************************************************************
typedef int (*LLD_StartProgress)( int start, int end );

#endif