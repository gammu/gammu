/* (c) 2001-2004 by Marcin Wiacek and Michal Cihar */

#ifndef __gsm_pbk_h
#define __gsm_pbk_h

#include <stdlib.h>

#include "../gsmcomon.h"
#include "gsmmisc.h"

/**
 * Structure contains info about number of used/free entries in phonebook
 * memory
 */
typedef struct {
	/**
	 * Number of used entries
	 */
	int	     	MemoryUsed;
	/**
	 * Memory type
	 */
	GSM_MemoryType  MemoryType;
	/**
	 * Number of free entries
	 */
	int	     	MemoryFree;
} GSM_MemoryStatus;

/**
 * Type of specific phonebook entry. In parenthesis is specified in which
 * member of @ref GSM_SubMemoryEntry value is stored.
 */
typedef enum {
	/**
	 * General number. (Text)
	 */
	PBK_Number_General = 1,
	/**
	 * Mobile number. (Text)
	 */
	PBK_Number_Mobile,
	/**
	 * Work number. (Text)
	 */
	PBK_Number_Work,
	/**
	 * Fax number. (Text)
	 */
	PBK_Number_Fax,
	/**
	 * Home number. (Text)
	 */
	PBK_Number_Home,
	/**
	 * Pager number. (Text)
	 */
	PBK_Number_Pager,
	/**
	 * Other number. (Text)
	 */
	PBK_Number_Other,
	/**
	 * Note. (Text)
	 */
	PBK_Text_Note,
	/**
	 * Complete postal address. (Text)
	 */
	PBK_Text_Postal,
	/**
	 * Email. (Text)
	 */
	PBK_Text_Email,
	/**
	 * Second email. (Text)
	 */
	PBK_Text_Email2,
	/**
	 * URL (Text)
	 */
	PBK_Text_URL,
	/**
	 * Date and time. FIXME: describe better (Date)
	 */
	PBK_Date,
	/**
	 * Caller group. (Text)
	 */
	PBK_Caller_Group,
	/**
	 * Name (Text)
	 */
	PBK_Text_Name,
	/**
	 * Last name. (Text)
	 */
	PBK_Text_LastName,
	/**
	 * First name. (Text)
	 */
	PBK_Text_FirstName,
	/**
	 * Company. (Text)
	 */
	PBK_Text_Company,
	/**
	 * Job title. (Text)
	 */
	PBK_Text_JobTitle,
	/**
	 * Category. (Number)
	 */
	PBK_Category,
	/**
	 * Whether entry is private. (Number)
	 */
	PBK_Private,
	/**
	 * Street address. (Text)
	 */
	PBK_Text_StreetAddress,
	/**
	 * City. (Text)
	 */
	PBK_Text_City,
	/**
	 * State. (Text)
	 */
	PBK_Text_State,
	/**
	 * Zip code. (Text)
	 */
	PBK_Text_Zip,
	/**
	 * Country. (Text)
	 */
	PBK_Text_Country,
	/**
	 * Custom information 1. (Text)
	 */
	PBK_Text_Custom1,
	/**
	 * Custom information 2. (Text)
	 */
	PBK_Text_Custom2,
	/**
	 * Custom information 3. (Text)
	 */
	PBK_Text_Custom3,
	/**
	 * Custom information 4. (Text)
	 */
	PBK_Text_Custom4,
	/**
	 * Ringtone ID. (Number)
	 */
	PBK_RingtoneID,
	/**
	 * Ringtone ID in phone filesystem. (Number)
	 */
	PBK_RingtoneFileSystemID,
	/**
	 * Picture ID. (Number)
	 */
	PBK_PictureID,
	PBK_SMSListID,
	/**
	 * User ID. (Text)
	 */
	PBK_Text_UserID
} GSM_EntryType;

#define GSM_PHONEBOOK_TEXT_LENGTH       200
#define GSM_PHONEBOOK_ENTRIES	   	26

/**
 * One value of phonebook memory entry.
 */
typedef struct {
	/**
	 * Type of entry.
	 */
	GSM_EntryType	   	EntryType;
	/**
	 * Text of entry (if applicable, see @ref GSM_EntryType).
	 */
	unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
	/**
	 * Text of entry (if applicable, see @ref GSM_EntryType).
	 */
	GSM_DateTime	    	Date;
	/**
	 * Number of entry (if applicable, see @ref GSM_EntryType).
	 */
	int		     	Number;
	/**
	 * Voice dialling tag.
	 */
	int		     	VoiceTag;
	int			SMSList[20];
} GSM_SubMemoryEntry;

/**
 * Structure for saving phonebook entries
 */
typedef struct {
	/**
	 * Used memory for phonebook entry
	 */
	GSM_MemoryType	  	MemoryType;
	/**
	 * Used location for phonebook entry
	 */
	int		     	Location;
	/**
	 * Number of SubEntries in Entries table.
	 */
	int		     	EntriesNum;
	/**
	 * Values of SubEntries.
	 */
	GSM_SubMemoryEntry      Entries[GSM_PHONEBOOK_ENTRIES];
} GSM_MemoryEntry;

typedef enum {
	Nokia_VCard10 = 1,
	Nokia_VCard21,
	SonyEricsson_VCard10,
	SonyEricsson_VCard21
} GSM_VCardVersion;

void GSM_PhonebookFindDefaultNameNumberGroup(GSM_MemoryEntry *entry, int *Name, int *Number, int *Group);
unsigned char *GSM_PhonebookGetEntryName (GSM_MemoryEntry *entry);

void      GSM_EncodeVCARD(char *Buffer, int *Length, GSM_MemoryEntry *pbk, bool header, GSM_VCardVersion Version);
GSM_Error GSM_DecodeVCARD(unsigned char *Buffer, int *Pos, GSM_MemoryEntry *Pbk, GSM_VCardVersion Version);

#ifndef ENABLE_LGPL
/* (c) by Timo Teras */
void DecodeVCARD21Text(char *VCard, GSM_MemoryEntry *pbk);
#endif

/**
 * Structure for saving speed dials
 */
typedef struct {
	/**
	 * Number of speed dial: 2,3..,8,9
	 */
	int	     		Location;
	/**
	 * ID of phone number used in phonebook entry
	 */
	int	     		MemoryNumberID;
	/**
	 * Memory, where is saved used phonebook entry
	 */
	GSM_MemoryType  	MemoryType;
	/**
	 * Location in memory, where is saved used phonebook entry
	 */
	int	     		MemoryLocation;
} GSM_SpeedDial;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
