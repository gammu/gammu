/* (c) 2001-2005 by Marcin Wiacek and Michal Cihar */
#ifndef _MCEMOBILEFUN_H
#define _MCEMOBILEFUN_H

#define GSM_PHONEBOOK_TEXT_LENGTH       200
#define GSM_PHONEBOOK_ENTRIES	   	26

typedef enum {
	// General number. (Text)
	PBK_Number_General = 1,

	// Mobile number. (Text)
	PBK_Number_Mobile,

	// Work number. (Text)
	PBK_Number_Work,

	// Fax number. (Text)
	PBK_Number_Fax,

	// Home number. (Text)
	PBK_Number_Home,

	// Pager number. (Text)
	PBK_Number_Pager,

	// Other number. (Text)
	PBK_Number_Other,

	// Note. (Text)
	PBK_Text_Note,

	// Complete postal address. (Text)
	PBK_Text_Postal,

	// Email. (Text)
	PBK_Text_Email,

	// Second email. (Text)
	PBK_Text_Email2,

	// URL (Text)
	PBK_Text_URL,

	// Date and time. FIXME: describe better (Date)
	PBK_Date,

	// Caller group. (Text)
	PBK_Caller_Group,

	// Name (Text)
	PBK_Text_Name,

	// Last name. (Text)
	PBK_Text_LastName,

	// First name. (Text)
	PBK_Text_FirstName,

	// Company. (Text)
	PBK_Text_Company,

	// Job title. (Text)
	PBK_Text_JobTitle,

	// Category. (Number)
	PBK_Category,

	// Whether entry is private. (Number)
	PBK_Private,

	// Street address. (Text)
	PBK_Text_StreetAddress,

	// City. (Text)
	PBK_Text_City,

	// State. (Text)
	PBK_Text_State,

	// Zip code. (Text)
	PBK_Text_Zip,

	// Country. (Text)
	PBK_Text_Country,

	// Custom information 1. (Text)
	PBK_Text_Custom1,

	// Custom information 2. (Text)
	PBK_Text_Custom2,

	// Custom information 3. (Text)
	PBK_Text_Custom3,

	// Custom information 4. (Text)
	PBK_Text_Custom4,

	// Ringtone ID. (Number)
	PBK_RingtoneID,

	// Ringtone ID in phone filesystem. (Number)
	PBK_RingtoneFileSystemID,

	// Picture ID. (Number)
	PBK_PictureID,
	PBK_SMSListID,

	// User ID. (Text)
	PBK_Text_UserID,

	// PictureName (Text)
	PBK_Text_Picture,

	// RingName (Text)
	PBK_Text_Ring,

	// Sex
	PBK_Number_Sex,

	// Sex
	PBK_Number_Light,
		// Caller group. (Text)

	//Compare : Push Talk. (TEXT) // Added by Mingfa 0127; 
    PBK_Push_Talk,


	PBK_Caller_Group_Text,

	// Street address. (Text)
	PBK_Text_StreetAddress2,

	//Nickname (Text)
	PBK_Text_Nickname,

	PBK_Number_Mobile_Home,
	PBK_Number_Mobile_Work,
	PBK_Number_Fax_Home,
	PBK_Number_Fax_Work,
	PBK_Text_Email_Home,
	PBK_Text_Email_Work,
	PBK_Text_URL_Home,
	PBK_Text_URL_Work,
	PBK_Text_Postal_Home,
	PBK_Text_Postal_Work,
	PBK_Number_Pager_Home,
	PBK_Number_Pager_Work,
	PBK_Number_VideoCall,
	PBK_Number_VideoCall_Home,
	PBK_Number_VideoCall_Work,
	PBK_Text_MiddleName,
	PBK_Text_Suffix,
	PBK_Text_Title,
	PBK_Text_Email_Mobile,
	PBK_Text_Email_Unknown,

//new add
	PBK_Number_Assistant ,			//64
	PBK_Number_Business ,			//65
	PBK_Number_Callback,			//66
	PBK_Number_Car,					//67
	PBK_Number_ISDN,				//68
	PBK_Number_Primary,				//69
	PBK_Number_Radio,				//70
	PBK_Number_Telix,				//71
	PBK_Number_TTYTDD,				//72

	PBK_Text_Department,			//73
	PBK_Text_Office,				//74
	PBK_Text_Profession,			//75
	PBK_Text_Manager_Name,			//76
	PBK_Text_Assistant_Name,		//77
	PBK_Text_Spouse_Name,			//78
	PBK_Date_Anniversary,			//79
	PBK_Text_Directory_Server,		//80
	PBK_Text_Email_alias,			//81
	PBK_Text_Internet_Address,		//82
	PBK_Text_Children,				//83

	PBK_Text_StreetAddress_Work,	//84
	PBK_Text_City_Work,				//85
	PBK_Text_State_Work,			//86
	PBK_Text_Zip_Work,				//87
	PBK_Text_Country_Work,			//88

	PBK_Text_StreetAddress_Home,	//89
	PBK_Text_City_Home,				//90
	PBK_Text_State_Home,			//91
	PBK_Text_Zip_Home,				//92
	PBK_Text_Country_Home,			//93
	PBK_Text_IMID					//94
} GSM_EntryType;

typedef struct {
	// The difference between local time and GMT in hours
	int			Timezone;

	unsigned int		Second;
	unsigned int 		Minute;
	unsigned int		Hour;

	unsigned int 		Day;

	// January = 1, February = 2, etc.
	unsigned int 		Month;

	// Complete year number. Not 03, but 2003
	unsigned int		Year;
} GSM_DateTime;

typedef enum {
	// Internal memory of the mobile equipment
	MEM_ERROR=0,
	MEM_ME=1,

	// SIM card memory
	MEM_SM,

	// Own numbers
	MEM_ON,

	// Dialled calls
	MEM_DC,

	// Received calls
	MEM_RC,

	// Missed calls
	MEM_MC,

	// Combined ME and SIM phonebook
	MEM_MT,

	// Fixed dial
	MEM_FD,

	// Voice mailbox
	MEM_VM,

	MEM7110_CG		 = 0xf0, /* Caller groups memory 		*/
	MEM7110_SP		 = 0xf1	 /* Speed dial memory 			*/
} GSM_MemoryType;
/**
 * One value of phonebook memory entry.
 */
typedef struct {
	// Type of entry.
	GSM_EntryType	   	EntryType;

	// Text of entry (if applicable, see @ref GSM_EntryType).
	unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];

	// Text of entry (if applicable, see @ref GSM_EntryType).
	GSM_DateTime	    	Date;

	// Number of entry (if applicable, see @ref GSM_EntryType).
	int		     	Number;

	// Voice dialling tag.
	int		     	VoiceTag;
	int			SMSList[20];
} GSM_SubMemoryEntry;


/**
 * Structure for saving phonebook entries
 */
typedef struct {
	// Used memory for phonebook entry
	GSM_MemoryType	  	MemoryType;

	// Used location for phonebook entry
//	int		     	Location;
	char			szIndex[100];

	// Number of SubEntries in Entries table.
	int		     	EntriesNum;

	// Values of SubEntries.
	GSM_SubMemoryEntry      Entries[GSM_PHONEBOOK_ENTRIES];
} GSM_MemoryEntry;

bool WINAPI MCEGetBeforeOpenMsg(TCHAR * pAppletName ,TCHAR* pMsg );
int WINAPI MCEOpenDriver();
bool WINAPI CheckMobileConnect();
bool WINAPI MCEGetInitMsg(TCHAR* pMsg );
void WINAPI MCEGetErrMsgStr(int iErr,TCHAR* pMsg);
//phonebook
int WINAPI MCE_PhoneBook_Init();
int WINAPI MCE_PhoneBook_GetInfo(char* MemoryType,int* used,int *free);
int WINAPI MCE_PhoneBook_GetData(char* MemoryType,GSM_MemoryEntry* contactEntry,bool bStart);
int WINAPI MCE_PhoneBook_DeleteData(char* MemoryType,char* szLocation, int Control);
int WINAPI MCE_PhoneBook_AddData(char* MemoryType,GSM_MemoryEntry* contactEntry, int Control);
int WINAPI MCE_PhoneBook_UpdateData(char* MemoryType,GSM_MemoryEntry* NewEntry,GSM_MemoryEntry* OldEntry);

#endif